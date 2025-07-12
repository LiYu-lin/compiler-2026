#include "GenIR/Visitor.h"
#include "Parse/AST.h"
#include "ir/basicblock.h"
#include "ir/type.h"
using BType = frontend::ast::ASTNode::BType;
namespace frontend::visitor {

BType Visitor::getBTypeFromAST(ast::ASTNode::BType btype) {
    switch (btype) {
        case ast::ASTNode::BType::Int: return BType::Int;
        case ast::ASTNode::BType::Float: return BType::Float;
        default: throw std::runtime_error("Unknown BType");
    }
}

IR::pType Visitor::getTypeFromBType(BType btype) {
    switch (btype) {
        case BType::Int: return IR::Type::getI32Type();
        case BType::Float: return IR::Type::getFloatType();
        default: throw std::runtime_error("Unknown BType");
    }
}

IR::Value* Visitor::visit(const ast::ASTNode &node) {
    return const_cast<ast::ASTNode&>(node).accept(*this);
}

IR::Value* Visitor::visit(const ast::CompUnit &node) {
    for (const auto &decl : node.decls) {
        if (!decl) {
            std::cerr << "Warning: null declaration in CompUnit" << std::endl;
            continue;
        }
        decl->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::Decl &node) {
    return node.decl->accept(*this);
}

IR::Value* Visitor::visit(const ast::ConstDecl &node) {
    for (const auto &def : node.constDefs) {
        def->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::VarDecl &node) {
    currentBType = node.btype;
    for (const auto &def : node.varDefs) {
        def->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::VarDef &node) {
    IR::pType varType = getTypeFromBType(currentBType);
    
    if (!node.dimensions.empty()) {
        std::vector<IR::Value*> dimValues;
        for (const auto& dim : node.dimensions) {
            dimValues.push_back(dim->accept(*this));
        }
        varType = IR::ArrayType::getArrayType(dimValues.size(), varType);
    }
    
    auto alloca = builder.CreateAlloca(varType, node.ident);

    if (node.initVal && *node.initVal) {
        auto initVal = (*node.initVal)->accept(*this);
        builder.CreateStore(initVal, alloca);
    }
    
    return alloca;
}

IR::Value* Visitor::visit(const ast::FuncDef &node) {
    symbolTable.enterScope();
    if (node.funcFParams) {
        node.funcFParams->accept(*this);
    }
    node.block->accept(*this);
    symbolTable.exitScope();
    return nullptr;
}

IR::Value* Visitor::visit(const ast::FuncFParams& node) {
    for (const auto& param : node.funcFParams) {
        param->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::FuncFParam& node) {
    IR::pType paramType = getTypeFromBType(node.btype);
    
    if (node.dimensions && !node.dimensions->empty()) {
        paramType = IR::ArrayType::getArrayType(node.dimensions->size(), paramType);
    }
    
    auto alloca = builder.CreateAlloca(paramType, node.ident);
    return alloca;
}

IR::Value* Visitor::visit(const ast::Block &node) {
    symbolTable.enterScope();
    for (const auto &item : node.blockItems) {
        item->accept(*this);
    }
    symbolTable.exitScope();
    return nullptr;
}

IR::Value* Visitor::visit(const ast::BlockItem &node) {
    return node.item->accept(*this);
}

IR::Value* Visitor::visit(const ast::Stmt &node) {
    return node.stmt->accept(*this);
}

IR::Value* Visitor::visit(const ast::Stmt::AssignStmt& node) {
    auto lval = node.lval->accept(*this);
    auto exp = node.exp->accept(*this);
    builder.CreateStore(exp, lval);
    return exp;
}

IR::Value* Visitor::visit(const ast::Stmt::ExpStmt& node) {

    if (node.exp && *node.exp) {
        return (*node.exp)->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::Stmt::BlockStmt& node) {
    return node.block->accept(*this);
}

IR::Value* Visitor::visit(const ast::Stmt::ContinueStmt& node) {
    if (!loopStack.empty()) {
        loop_info& loop = loopStack.top();
        return builder.CreateBr(loop.continue_b);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::Stmt::ReturnStmt& node) {
    if (node.exp && *node.exp) {
        auto retVal = (*node.exp)->accept(*this);
        return builder.CreateRet(retVal, currentFunction);
    } else {
        return builder.CreateRetVoid(currentFunction);
    }
}

IR::Value* Visitor::visit(const ast::Stmt::IfStmt &node) {
    auto cond = node.exp->accept(*this);
    auto trueBB = new IR::BasicBlock("if.true");
    auto falseBB = new IR::BasicBlock("if.false");
    auto mergeBB = new IR::BasicBlock("if.merge");

    auto condBr = builder.CreateCondBr(cond, trueBB, falseBB);
    
    builder.SetInsertPoint(trueBB);
    node.block->accept(*this);
    builder.CreateBr(mergeBB);
    
    if (node.elseStmt) {
        builder.SetInsertPoint(falseBB);
        (*node.elseStmt)->accept(*this);
        builder.CreateBr(mergeBB);
    }
    
    builder.SetInsertPoint(mergeBB);
    return condBr;
}

IR::Value* Visitor::visit(const ast::Stmt::WhileStmt &node) {
    loop_info loop;
    loop.continue_b = new IR::BasicBlock("continue");
    loop.break_b = new IR::BasicBlock("break");
    loopStack.push(loop);
    
    auto cond = node.cond->accept(*this);
    node.stmt->accept(*this);
    
    loopStack.pop();
    return cond;
}

IR::Value* Visitor::visit(const ast::Stmt::BreakStmt &node) {
    if (!loopStack.empty()) {
        loop_info &loop = loopStack.top();
        return builder.CreateBr(loop.break_b);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::Number &node) {
    if (std::holds_alternative<int>(node.value)) {
        return IR::ConstantInt32::get(std::get<int>(node.value));
    } else {
        return IR::ConstantFloat::get(std::get<float>(node.value));
    }
}

IR::Value* Visitor::visit(const ast::LVal &node) {
    auto symbol = symbolTable.lookup(node.ident);
    if (!symbol) return undefinedValue;
    
    IR::Value *varPtr = symbol->value;
    IR::pType varType = getTypeFromBType(symbol->baseType);

    if (!node.dimensions.empty()) {
        std::vector<IR::Value*> indices;
        indices.push_back(IR::ConstantInt32::get(0)); 
        for (const auto& dim : node.dimensions) {
            indices.push_back(dim->accept(*this));
        }
        varPtr = builder.CreateGEP(varType, varPtr, indices);
    }
    
    return builder.CreateLoad(varType, varPtr);
}

IR::Value* Visitor::visit(const ast::Exp& node) {
    return node.addExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::PrimaryExp& node) {
    if (std::holds_alternative<ast::ASTNodePtr>(node.primaryExp)) {
        return std::get<ast::ASTNodePtr>(node.primaryExp)->accept(*this);
    } else {
        return visit(std::get<ast::Number>(node.primaryExp));
    }
}

IR::Value* Visitor::visit(const ast::UnaryExp &node) {
    return node.unaryExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::UnaryExp::UnaryExpCall& node) {
    std::vector<IR::Value*> args;
    for (auto& param : node.funcRParams) {
        args.push_back(param->accept(*this));
    }
    auto funcSymbol = symbolTable.lookup(node.ident);
    if (!funcSymbol || !funcSymbol->value->isFunction()) {
        return undefinedValue;
    }
    auto callee = static_cast<IR::Function*>(funcSymbol->value);
    return builder.CreateCall(callee, args);
}

IR::Value* Visitor::visit(const ast::UnaryExp::UnaryExpOp &node) {
    auto val = node.unaryExp->accept(*this);
    switch(node.unaryOp) {
        case ast::ASTNode::UnaryOp::Plus: return val;
        case ast::ASTNode::UnaryOp::Minus: 
            return builder.CreateNeg(val);
        case ast::ASTNode::UnaryOp::Not:
            return builder.CreateNot(val);
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(const ast::FuncRParams& node) {
    std::vector<IR::Value*> args;
    for (auto& exp : node.exps) {
        args.push_back(exp->accept(*this));
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(const ast::MulExp &node) {
    return node.mulExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::MulExp::MulExpOp& node) {
    auto left = node.mulExp->accept(*this);
    auto right = node.unaryExp->accept(*this);
    switch(node.mulOp) {
        case ast::ASTNode::MulOp::Mul: 
            return builder.CreateMul(left, right);
        case ast::ASTNode::MulOp::Div:
            return builder.CreateDiv(left, right);
        case ast::ASTNode::MulOp::Mod:
            return builder.CreateRem(left, right);
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(const ast::AddExp &node) {
    return node.addExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::AddExp::AddExpOp& node) {
    auto left = node.addExp->accept(*this);
    auto right = node.mulExp->accept(*this);
    switch(node.addOp) {
        case ast::ASTNode::AddOp::Plus:
            return builder.CreateAdd(left, right);
        case ast::ASTNode::AddOp::Minus:
            return builder.CreateSub(left, right);
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(const ast::RelExp &node) {
    return node.relExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::RelExp::RelExpOp& node) {
    auto left = node.relExp->accept(*this);
    auto right = node.addExp->accept(*this);
    
    if (left->getType()->isFloatTy() != right->getType()->isFloatTy()) {
        if (left->getType()->isFloatTy()) {
            right = builder.CreateSItoFP(right);
        } else {
            left = builder.CreateSItoFP(left);
        }
    }

    if (left->getType()->isFloatTy()) {
        switch(node.relOp) {
            case ast::ASTNode::RelOp::Less:
                return builder.CreateFLt(left, right);
            case ast::ASTNode::RelOp::Greater:
                return builder.CreateFGt(left, right);
            case ast::ASTNode::RelOp::LessEqual:
                return builder.CreateFLe(left, right);
            case ast::ASTNode::RelOp::GreaterEqual:
                return builder.CreateFGe(left, right);
        }
    } else {
        switch(node.relOp) {
            case ast::ASTNode::RelOp::Less:
                return builder.CreateLt(left, right);
            case ast::ASTNode::RelOp::Greater:
                return builder.CreateGt(left, right);
            case ast::ASTNode::RelOp::LessEqual:
                return builder.CreateLe(left, right);
            case ast::ASTNode::RelOp::GreaterEqual:
                return builder.CreateGe(left, right);
        }
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(const ast::EqExp &node) {
    return node.eqExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::EqExp::EqExpOp& node) {
    auto left = node.eqExp->accept(*this);
    auto right = node.relExp->accept(*this);
    
    if (left->getType()->isFloatTy() != right->getType()->isFloatTy()) {
        if (left->getType()->isFloatTy()) {
            right = builder.CreateSItoFP(right);
        } else {
            left = builder.CreateSItoFP(left);
        }
    }

    if (left->getType()->isFloatTy()) {
        switch(node.eqOp) {
            case ast::ASTNode::EqOp::Equal:
                return builder.CreateFEq(left, right);
            case ast::ASTNode::EqOp::NotEqual:
                return builder.CreateFNe(left, right);
        }
    } else {
        switch(node.eqOp) {
            case ast::ASTNode::EqOp::Equal:
                return builder.CreateEq(left, right);
            case ast::ASTNode::EqOp::NotEqual:
                return builder.CreateNe(left, right);
        }
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(const ast::LAndExp &node) {
    return node.lAndExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::LAndExp::LAndExpOp& node) {
    auto left = node.lAndExp->accept(*this);
    auto right = node.eqExp->accept(*this);
    return builder.CreateAnd(left, right);
}

IR::Value* Visitor::visit(const ast::LOrExp &node) {
    return node.lOrExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::LOrExp::LOrExpOp& node) {
    auto left = node.lOrExp->accept(*this);
    auto right = node.lAndExp->accept(*this);
    return builder.CreateOr(left, right);
}

IR::Value* Visitor::visit(const ast::ConstExp& node) {
    return node.addExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::ConstDef& node) {
    IR::pType constType = getTypeFromBType(currentBType);
    
    if (!node.dimensions.empty()) {
        std::vector<IR::Value*> dimValues;
        for (auto& dim : node.dimensions) {
            dimValues.push_back(dim->accept(*this));
        }
        constType = IR::ArrayType::getArrayType(dimValues.size(), constType);
    }
    
    auto alloca = builder.CreateAlloca(constType, node.ident);
    
    if (node.constInitVal) {
        auto initVal = node.constInitVal->accept(*this);
        builder.CreateStore(initVal, alloca);
    }
    
    return alloca;
}

IR::Value* Visitor::visit(const ast::InitVal& node) {
    if (std::holds_alternative<ast::ASTNodePtr>(node.initVal)) {
        return std::get<ast::ASTNodePtr>(node.initVal)->accept(*this);
    } else {
        return undefinedValue;
    }
}

IR::Value* Visitor::visit(const ast::ConstInitVal& node) {
    if (std::holds_alternative<ast::ASTNodePtr>(node.constInitVal)) {
        return std::get<ast::ASTNodePtr>(node.constInitVal)->accept(*this);
    } else {
        return undefinedValue;
    }
}

IR::Value* Visitor::visit(const ast::Cond& node) {
    return node.lOrExp->accept(*this);
}

} // namespace frontend::visitor