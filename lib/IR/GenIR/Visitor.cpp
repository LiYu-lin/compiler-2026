#include "SymbolTable/Visitor.h"
#include "Parse/AST.h"
#include "ir/basicblock.h"
#include "ir/type.h"
using BType = frontend::ast::ASTNode::BType;
namespace frontend::visitor {

BType getBTypeFromAST(ast::ASTNode::BType btype) {
    switch (btype) {
        case ast::ASTNode::BType::Int: return BType::Int;
        case ast::ASTNode::BType::Float: return BType::Float;
        default: throw std::runtime_error("Unknown BType");
    }
}

IR::pType getTypeFromBType(BType btype) {
    switch (btype) {
        case BType::Int: return IR::Type::getI32Type();
        case BType::Float: return IR::Type::getFloatType();
        default: throw std::runtime_error("Unknown BType");
    }
}

IR::Value* Visitor::visit(const ast::ASTNode &node) {
    switch(node.type) {
        case ast::ASTNode::Type::CompUnit:
            return visit(static_cast<const ast::CompUnit&>(node));
        default:
            return undefinedValue;
    }
}

IR::Value* Visitor::visit(ast::CompUnit &node) {
    for (auto &decl : node.decls) {
        visit(*decl);
    }
    return nullptr;
}

IR::Value* Visitor::visit(ast::Decl &node) {
    return visit(*node.decl);
}

IR::Value* Visitor::visit(ast::ConstDecl &node) {
    for (auto &def : node.constDefs) {
        visit(*def);
    }
    return nullptr;
}
IR::Value* Visitor::visit(ast::VarDecl &node) {
    // 保存当前类型到成员变量
    currentBType = node.btype;
    for (auto &def : node.varDefs) {
        visit(*def);
    }
    return nullptr;
}

IR::Value* Visitor::visit(ast::VarDef &node) {
    IR::pType varType = getTypeFromBType(currentBType);
    
    if (!node.dimensions.empty()) {
        std::vector<IR::Value*> dimValues;
        for (auto& dim : node.dimensions) {
            dimValues.push_back(visit(*dim));
        }
        varType = IR::ArrayType::getArrayType(dimValues.size(), varType);
    }
    
    auto alloca = builder.CreateAlloca(varType, node.ident);

    if (node.initVal && *node.initVal) {
        auto initVal = visit(**node.initVal);
        builder.CreateStore(initVal, alloca);
    }
    
    return alloca;
}

IR::Value* Visitor::visit(ast::FuncDef &node) {
    symbolTable.enterScope();
    if (node.funcFParams) {
        visit(*node.funcFParams);
    }
    visit(*node.block);
    symbolTable.exitScope();
    return nullptr;
}

IR::Value* Visitor::visit(ast::FuncFParams& node) {
    for (auto& param : node.funcFParams) {
        visit(*param);
    }
    return nullptr;
}

IR::Value* Visitor::visit(ast::FuncFParam& node) {
    IR::pType paramType = getTypeFromBType(node.btype);
    
    if (node.dimensions && !node.dimensions->empty()) {
        paramType = IR::ArrayType::getArrayType(node.dimensions->size(), paramType);
    }
    
    auto alloca = builder.CreateAlloca(paramType, node.ident);
    return alloca;
}

IR::Value* Visitor::visit(ast::Block &node) {
    symbolTable.enterScope();
    for (auto &item : node.blockItems) {
        visit(*item);
    }
    symbolTable.exitScope();
    return nullptr; // 无实际返回值
}

IR::Value* Visitor::visit(ast::BlockItem &node) {
    return visit(*node.item); 
}

IR::Value* Visitor::visit(ast::Stmt &node) {
    return visit(*node.stmt);
}

IR::Value* Visitor::visit(ast::Stmt::AssignStmt& node) {
    auto lval = visit(*node.lval);
    auto exp = visit(*node.exp);
    builder.CreateStore(exp, lval);
    return exp;
}

IR::Value* Visitor::visit(ast::Stmt::ExpStmt& node) {
    if (node.exp && *node.exp) {
        return visit(**node.exp);
    }
    return nullptr;
}

IR::Value* Visitor::visit(ast::Stmt::BlockStmt& node) {
    return visit(*node.block);
}

IR::Value* Visitor::visit(ast::Stmt::ContinueStmt& node) {
    if (!loopStack.empty()) {
        loop_info& loop = loopStack.top();
        return builder.CreateBr(loop.continue_b);
    }
    return nullptr;
}

IR::Value* Visitor::visit(ast::Stmt::ReturnStmt& node) {
    if (node.exp && *node.exp) {
        auto retVal = visit(**node.exp);
        return builder.CreateRet(retVal,currentFunction);
    } else {
        return builder.CreateRetVoid(currentFunction);
    }
}

IR::Value* Visitor::visit(ast::Stmt::IfStmt &node) {
    auto cond = visit(*node.exp);
    auto trueBB = new IR::BasicBlock("if.true");
    auto falseBB = new IR::BasicBlock("if.false");
    auto mergeBB = new IR::BasicBlock("if.merge");

    auto condBr = builder.CreateCondBr(cond, trueBB, falseBB);
    
    builder.SetInsertPoint(trueBB);
    visit(*node.block);
    builder.CreateBr(mergeBB);
    
    if (node.elseStmt) {
        builder.SetInsertPoint(falseBB);
        visit(**node.elseStmt);
        builder.CreateBr(mergeBB);
    }
    
    builder.SetInsertPoint(mergeBB);
    return condBr; // 返回条件分支指令
}

IR::Value* Visitor::visit(ast::Stmt::WhileStmt &node) {
    loop_info loop;
    loop.continue_b = new IR::BasicBlock("continue");
    loop.break_b = new IR::BasicBlock("break");
    loopStack.push(loop);
    
    auto cond = visit(*node.cond);
    visit(*node.stmt);
    
    loopStack.pop();
    return cond; // 返回条件表达式值
}

IR::Value* Visitor::visit(ast::Stmt::BreakStmt &node) {
    if (!loopStack.empty()) {
        loop_info &loop = loopStack.top();
        return builder.CreateBr(loop.break_b);
    }
    return nullptr;
}

IR::Value* Visitor::visit(ast::Number &node) {
    if (std::holds_alternative<int>(node.value)) {
        return IR::ConstantInt32::get(std::get<int>(node.value));
    } else {
        return IR::ConstantFloat::get(std::get<float>(node.value));
    }
}

IR::Value* Visitor::visit(ast::LVal &node) {
    auto symbol = symbolTable.lookup(node.ident);
    if (!symbol) return undefinedValue;
    
    IR::Value *varPtr = symbol->value;
    IR::pType varType = getTypeFromBType(symbol->baseType);

    if (!node.dimensions.empty()) {
        std::vector<IR::Value*> indices;
        indices.push_back(IR::ConstantInt32::get(0)); 
        for (auto& dim : node.dimensions) {
            indices.push_back(visit(*dim));
        }
        varPtr = builder.CreateGEP(varType, varPtr, indices);
    }
    
    return builder.CreateLoad(varType, varPtr);
}

IR::Value* Visitor::visit(ast::Exp& node) {
    return visit(*node.addExp);
}

IR::Value* Visitor::visit(ast::PrimaryExp& node) {
    if (std::holds_alternative<ast::ASTNodePtr>(node.primaryExp)) {
        return visit(*std::get<ast::ASTNodePtr>(node.primaryExp));
    } else {
        return visit(std::get<ast::Number>(node.primaryExp));
    }
}

IR::Value* Visitor::visit(ast::UnaryExp &node) {
    return visit(*node.unaryExp);
}

IR::Value* Visitor::visit(ast::UnaryExp::UnaryExpCall& node) {
    std::vector<IR::Value*> args;
    for (auto& param : node.funcRParams) {
        args.push_back(visit(*param));
    }
    auto funcSymbol = symbolTable.lookup(node.ident);
    if (!funcSymbol || !funcSymbol->value->isFunction()) {
        return undefinedValue;
    }
    auto callee = static_cast<IR::Function*>(funcSymbol->value);
    return builder.CreateCall(callee, args);
}

IR::Value* Visitor::visit(ast::UnaryExp::UnaryExpOp &node) {
    auto val = visit(*node.unaryExp);
    switch(node.unaryOp) {
        case ast::ASTNode::UnaryOp::Plus: return val;
        case ast::ASTNode::UnaryOp::Minus: 
            return builder.CreateNeg(val);
        case ast::ASTNode::UnaryOp::Not:
            return builder.CreateNot(val);
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(ast::FuncRParams& node) {
    std::vector<IR::Value*> args;
    for (auto& exp : node.exps) {
        args.push_back(visit(*exp));
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(ast::MulExp &node) {
    return visit(*node.mulExp);
}

IR::Value* Visitor::visit(ast::MulExp::MulExpOp& node) {
    auto left = visit(*node.mulExp);
    auto right = visit(*node.unaryExp);
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

IR::Value* Visitor::visit(ast::AddExp &node) {
    return visit(*node.addExp);
}

IR::Value* Visitor::visit(ast::AddExp::AddExpOp& node) {
    auto left = visit(*node.addExp);
    auto right = visit(*node.mulExp);
    switch(node.addOp) {
        case ast::ASTNode::AddOp::Plus:
            return builder.CreateAdd(left, right);
        case ast::ASTNode::AddOp::Minus:
            return builder.CreateSub(left, right);
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(ast::RelExp &node) {
    return visit(*node.relExp);
}

IR::Value* Visitor::visit(ast::RelExp::RelExpOp& node) {
    auto left = visit(*node.relExp);
    auto right = visit(*node.addExp);
    
    // 类型转换处理
    if (left->getType()->isFloatTy() != right->getType()->isFloatTy()) {
        if (left->getType()->isFloatTy()) {
            right = builder.CreateSItoFP(right);
        } else {
            left = builder.CreateSItoFP(left);
        }
    }

    if (left->getType()->isFloatTy()) {
        // 浮点比较
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
        // 整数比较
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

IR::Value* Visitor::visit(ast::EqExp &node) {
    return visit(*node.eqExp);
}

IR::Value* Visitor::visit(ast::EqExp::EqExpOp& node) {
    auto left = visit(*node.eqExp);
    auto right = visit(*node.relExp);
    
    // 类型转换处理
    if (left->getType()->isFloatTy() != right->getType()->isFloatTy()) {
        if (left->getType()->isFloatTy()) {
            right = builder.CreateSItoFP(right);
        } else {
            left = builder.CreateSItoFP(left);
        }
    }

    if (left->getType()->isFloatTy()) {
        // 浮点比较
        switch(node.eqOp) {
            case ast::ASTNode::EqOp::Equal:
                return builder.CreateFEq(left, right);
            case ast::ASTNode::EqOp::NotEqual:
                return builder.CreateFNe(left, right);
        }
    } else {
        // 整数比较
        switch(node.eqOp) {
            case ast::ASTNode::EqOp::Equal:
                return builder.CreateEq(left, right);
            case ast::ASTNode::EqOp::NotEqual:
                return builder.CreateNe(left, right);
        }
    }
    return undefinedValue;
}

IR::Value* Visitor::visit(ast::LAndExp &node) {
    return visit(*node.lAndExp);
}

IR::Value* Visitor::visit(ast::LAndExp::LAndExpOp& node) {
    auto left = visit(*node.lAndExp);
    auto right = visit(*node.eqExp);
    return builder.CreateAnd(left, right);
}

IR::Value* Visitor::visit(ast::LOrExp &node) {
    return visit(*node.lOrExp);
}

IR::Value* Visitor::visit(ast::LOrExp::LOrExpOp& node) {
    auto left = visit(*node.lOrExp);
    auto right = visit(*node.lAndExp);
    return builder.CreateOr(left, right);
}

IR::Value* Visitor::visit(ast::ConstExp& node) {
    return visit(*node.addExp);
}

IR::Value* Visitor::visit(ast::ConstDef& node) {
    // 处理常量定义
    IR::pType constType = getTypeFromBType(currentBType);
    
    if (!node.dimensions.empty()) {
        std::vector<IR::Value*> dimValues;
        for (auto& dim : node.dimensions) {
            dimValues.push_back(visit(*dim));
        }
        constType = IR::ArrayType::getArrayType(dimValues.size(), constType);
    }
    
    auto alloca = builder.CreateAlloca(constType, node.ident);
    
    if (node.constInitVal) {
        auto initVal = visit(*node.constInitVal);
        builder.CreateStore(initVal, alloca);
    }
    
    return alloca;
}

IR::Value* Visitor::visit(ast::InitVal& node) {
    if (std::holds_alternative<ast::ASTNodePtr>(node.initVal)) {
        return visit(*std::get<ast::ASTNodePtr>(node.initVal));
    } else {
        return undefinedValue;
    }
}

IR::Value* Visitor::visit(ast::ConstInitVal& node) {
    if (std::holds_alternative<ast::ASTNodePtr>(node.constInitVal)) {
        return visit(*std::get<ast::ASTNodePtr>(node.constInitVal));
    } else {
        return undefinedValue;
    }
}

IR::Value* Visitor::visit(ast::Cond& node) {
    return visit(*node.lOrExp);
}




} // namespace frontend::visitor