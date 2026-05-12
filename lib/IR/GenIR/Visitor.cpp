#include "Visitor.h"
#include "AST.h"
#include "globalvalue.h"
#include "basicblock.h"
#include "type.h"
#include "Value.h"
#include <iostream>
using BType = frontend::ast::ASTNode::BType;
namespace frontend::visitor {

BType Visitor::getBTypeFromAST(ast::ASTNode::BType btype) {
    switch (btype) {
        case ast::ASTNode::BType::Int: return BType::Int;
        case ast::ASTNode::BType::Float: return BType::Float;
        default: throw std::runtime_error("getBTypeFromAST: Unknown BType");
    }
}

IR::pType Visitor::getTypeFromBType(BType btype) {
              
    switch (btype) {
        case BType::Int: 
            return IR::Type::getI32Type();
        case BType::Float: 
            return IR::Type::getFloatType();
        default: {
            std::stringstream ss;
            ss << "getTypeFromBType: Unknown BType value: " << static_cast<int>(btype);
            throw std::runtime_error(ss.str());
        }
    }
}

IR::pType Visitor::getTypeFromfuncType(const FuncType &funcType) {
    switch (funcType) {
        case FuncType::Void: return IR::Type::getVoidType();
        case FuncType::Int: return IR::Type::getI32Type();
        case FuncType::Float: return IR::Type::getFloatType();
        default: throw std::runtime_error("getTypeFromfuncType: Unknown FuncType");
    }
}

IR::Value* Visitor::visit(const ast::ASTNode &node) {
    return const_cast<ast::ASTNode&>(node).accept(*this);
}

IR::Value* Visitor::getLValPointer(const ast::LVal &node) {
    auto symbol = symbolTable.lookup(node.ident);
    if (!symbol) {
        std::cerr << "Error: Undefined variable '" << node.ident << "'" << std::endl;
        return undefinedValue;
    }

    IR::Value *varPtr = symbol->value;
    IR::pType varType = getTypeFromBType(symbol->baseType);

    if (!node.dimensions.empty()) {
        if (!symbol->isArray) {
            std::cerr << "Error: '" << node.ident << "' is not an array" << std::endl;
            return undefinedValue;
        }
        std::vector<IR::Value*> indices;
        indices.push_back(IR::ConstantInt32::get(0));
        for (const auto& dim : node.dimensions) {
            indices.push_back(dim->accept(*this));
        }
        varPtr = builder.CreateGEP(varType, varPtr, indices);
    }

    return varPtr;
}

IR::Value* Visitor::coerceToBoolValue(IR::Value *value) {
    if (!value) {
        return undefinedValue;
    }

    if (value->isInstruction()) {
        auto *inst = static_cast<IR::Instruction *>(value);
        auto opcode = inst->getOpcode();
        if (opcode >= IR::Instruction::CmpBegin &&
            opcode < IR::Instruction::CmpEnd) {
            return value;
        }
    }

    if (value->getType()->isFloatTy()) {
        return builder.CreateFNe(value, IR::ConstantFloat::get(0.0f));
    }

    if (value->getType()->isInt32Ty()) {
        return builder.CreateNe(value, IR::ConstantInt32::get(0));
    }

    return value;
}

bool Visitor::currentBlockHasTerminator() const {
    auto *block = builder.BB;
    if (!block) {
        return false;
    }

    auto instructions = block->getVectorInstructions();
    if (instructions.empty()) {
        return false;
    }

    auto *last = instructions.back();
    return last->getOpcode() == IR::Instruction::BR ||
           last->getOpcode() == IR::Instruction::Return;
}

void Visitor::emitConditionalBranch(const ast::ASTNode &node, IR::BasicBlock *trueBB, IR::BasicBlock *falseBB) {
    if (auto cond = dynamic_cast<const ast::Cond*>(&node)) {
        emitConditionalBranch(*cond->lOrExp, trueBB, falseBB);
        return;
    }

    if (auto lor = dynamic_cast<const ast::LOrExp*>(&node)) {
        emitConditionalBranch(*lor->lOrExp, trueBB, falseBB);
        return;
    }

    if (auto lorOp = dynamic_cast<const ast::LOrExp::LOrExpOp*>(&node)) {
        auto rhsBB = new IR::BasicBlock("lor.rhs");
        if (currentFunction) {
            currentFunction->addBlock(rhsBB);
        }
        emitConditionalBranch(*lorOp->lOrExp, trueBB, rhsBB);
        builder.SetInsertPoint(rhsBB);
        emitConditionalBranch(*lorOp->lAndExp, trueBB, falseBB);
        return;
    }

    if (auto land = dynamic_cast<const ast::LAndExp*>(&node)) {
        emitConditionalBranch(*land->lAndExp, trueBB, falseBB);
        return;
    }

    if (auto landOp = dynamic_cast<const ast::LAndExp::LAndExpOp*>(&node)) {
        auto rhsBB = new IR::BasicBlock("land.rhs");
        if (currentFunction) {
            currentFunction->addBlock(rhsBB);
        }
        emitConditionalBranch(*landOp->lAndExp, rhsBB, falseBB);
        builder.SetInsertPoint(rhsBB);
        emitConditionalBranch(*landOp->eqExp, trueBB, falseBB);
        return;
    }

    auto condValue = coerceToBoolValue(visit(node));
    builder.CreateCondBr(condValue, trueBB, falseBB);
}

IR::Value* Visitor::visit(const ast::CompUnit &node) {
    auto globalInitBB = new IR::BasicBlock("global.init");
    builder.SetInsertPoint(globalInitBB);
    for (const auto &decl : node.decls) {
        if (!decl) {
            std::cerr << "Warning: null declaration in CompUnit" << std::endl;
            continue;
        }
        decl->accept(*this);
    }
    if (globalInitBB->getVectorInstructions().size() > 0) {
        module.addGlobalInitBlock(globalInitBB);
    } else {
        delete globalInitBB;
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::Decl &node) {
    return node.decl->accept(*this);
}

IR::Value* Visitor::visit(const ast::ConstDecl &node) {
    currentBType = node.btype;
    for (const auto &def : node.constDefs) {
        def->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::InitVal& node) {
    if (std::holds_alternative<ast::ASTNodePtr>(node.initVal)) {
        return std::get<ast::ASTNodePtr>(node.initVal)->accept(*this);
    } else {
        return undefinedValue;
    }
}

IR::Value* Visitor::visit(const ast::ConstDef& node) {
    
    // 检查常量是否已定义
    if (symbolTable.lookup(node.ident)) {
        std::cerr << "Error: Redefinition of constant '" << node.ident << "'" << std::endl;
        return undefinedValue;
    }

    IR::pType constType = getTypeFromBType(currentBType);
    std::vector<int> dimValues;
    if (!node.dimensions.empty()) {
        std::vector<IR::Value*> dimIRValues;
        for (auto& dim : node.dimensions) {
            dimIRValues.push_back(dim->accept(*this));
            if (auto constInt = dynamic_cast<IR::ConstantInt32*>(dimIRValues.back())) {
                dimValues.push_back(constInt->getValue());
            }
        }
        constType = IR::ArrayType::getArrayType(dimValues.size(), constType);
    }

    IR::Value* constValue = nullptr;
    if (!currentFunction) {  // 全局常量处理
        IR::Constant* initVal = nullptr;
        if (node.constInitVal) {
            auto val = node.constInitVal->accept(*this);
            initVal = dynamic_cast<IR::Constant*>(val);
            if (!initVal) {
                std::cerr << "Error: Global const must be initialized with constant" << std::endl;
                return undefinedValue;
            }
        }
        constValue = new IR::GlobalVariable(constType, node.ident, initVal, true); // true表示是常�?
        module.addGlobal(static_cast<IR::GlobalVariable*>(constValue));
    } else {  // 局部常量处�?
        constValue = builder.CreateAlloca(constType, node.ident, true);
        if (node.constInitVal) {
            auto initVal = node.constInitVal->accept(*this);
            builder.CreateStore(initVal, constValue);
        }
    }

    // 设置符号表信�?
    SymbolInfo info(node.ident, currentBType, true, symbolTable.getCurrentScopeLevel());
    info.isArray = !node.dimensions.empty();
    info.dims = dimValues;
    info.value = constValue;
    
    if (!symbolTable.insert(node.ident, info)) {
        std::cerr << "Error: Failed to insert constant '" << node.ident << "' into symbol table" << std::endl;
    }
    
    return constValue;
}

IR::Value* Visitor::visit(const ast::ConstInitVal& node) {
    if (std::holds_alternative<ast::ASTNodePtr>(node.constInitVal)) {
        auto ptr = std::get<ast::ASTNodePtr>(node.constInitVal);
        auto val = ptr->accept(*this);
        if (!dynamic_cast<IR::Constant*>(val)) {
            std::cerr << "Error: ConstInitVal must evaluate to constant" << std::endl;
            return undefinedValue;
        }
        return val;
    } else {
        return undefinedValue;
    }
}

IR::Value* Visitor::visit(const ast::Number &node) {
    if (std::holds_alternative<int>(node.value)) {
        int val = std::get<int>(node.value);
        return IR::ConstantInt32::get(val);
    } else {
        float val = std::get<float>(node.value);
        return IR::ConstantFloat::get(val);
    }
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
    
    std::vector<int> dimValues;
    if (!node.dimensions.empty()) {
        std::vector<IR::Value*> dimIRValues;
        for (const auto& dim : node.dimensions) {
            auto dimVal = dim->accept(*this);
            dimIRValues.push_back(dimVal);
            if (auto constInt = dynamic_cast<IR::ConstantInt32*>(dimVal)) {
                dimValues.push_back(constInt->getValue());
            } else {
                std::cerr << "Error: Array dimension must be constant integer" << std::endl;
                return undefinedValue;
            }
        }
        varType = IR::ArrayType::getArrayType(dimValues.size(), varType);
    }
    
    // 区分全局变量和局部变�?
    IR::Value* varValue = nullptr;
    if (currentFunction) {
        varValue = builder.CreateAlloca(varType, node.ident);
        
        if (node.initVal && *node.initVal) {
            auto initVal = (*node.initVal)->accept(*this);
            builder.CreateStore(initVal, varValue);
        }
    } else {
        // 全局变量处理
        IR::Constant* initVal = nullptr;
        if (node.initVal && *node.initVal) {
            auto val = (*node.initVal)->accept(*this);
            initVal = dynamic_cast<IR::Constant*>(val);
            if (!initVal) {
                std::cerr << "Error: Global variable must be initialized with constant" << std::endl;
                return undefinedValue;
            }
        }
        varValue = new IR::GlobalVariable(varType, node.ident, initVal, false);
        module.addGlobal(static_cast<IR::GlobalVariable*>(varValue));
    }

    // 符号表记�?
    SymbolInfo info(
        node.ident,
        currentBType,
        false,
        symbolTable.getCurrentScopeLevel()
    );
    info.isArray = !node.dimensions.empty();
    info.dims = dimValues;
    info.value = varValue;
    
    if (!symbolTable.insert(node.ident, info)) {
        std::cerr << "Error: Redefinition of variable '" << node.ident << "'" << std::endl;
    }
    
    return varValue;
}

IR::Value* Visitor::visit(const ast::FuncDef &node) {
    
    if (symbolTable.lookup(node.ident)) {
        std::cerr << "Error: Redefinition of function '" << node.ident << "'" << std::endl;
        return nullptr;
    }

    IR::pType retType = getTypeFromfuncType(node.funcType);
    std::vector<IR::pType> paramTypes;
    if (node.funcFParams) {
        for (const auto& param : static_cast<ast::FuncFParams*>(node.funcFParams.get())->funcFParams) {
            auto paramNode = static_cast<ast::FuncFParam*>(param.get());
            paramTypes.push_back(getTypeFromBType(paramNode->btype));
        }
    }
    currentFunction = new IR::Function( retType,node.ident ,paramTypes);
    module.addGlobal(currentFunction);
    
    SymbolInfo funcInfo(node.ident, node.funcType, symbolTable.getCurrentScopeLevel());
    funcInfo.value = currentFunction;
    symbolTable.insert(node.ident, funcInfo);
    
    symbolTable.enterScope();
    currentParamIndex = 0;
    auto entryBB = new IR::BasicBlock("entry");
    currentFunction->addBlock(entryBB);
    builder.SetInsertPoint(entryBB);
    
    if (node.funcFParams) {
        node.funcFParams->accept(*this);
    }
    
    node.block->accept(*this);
    
    currentFunction = nullptr;
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

    if (!currentFunction || currentParamIndex >= static_cast<size_t>(currentFunction->getArgCount())) {
        std::cerr << "Error: parameter lowering out of sync for '" << node.ident << "'" << std::endl;
        return undefinedValue;
    }

    auto *argValue = currentFunction->getArg(static_cast<unsigned int>(currentParamIndex++));
    auto *alloca = builder.CreateAlloca(paramType, node.ident);
    builder.CreateStore(argValue, alloca);

    SymbolInfo info(node.ident, node.btype, false, symbolTable.getCurrentScopeLevel());
    info.isArray = node.dimensions && !node.dimensions->empty();
    info.value = alloca;
    if (!symbolTable.insert(node.ident, info)) {
        std::cerr << "Error: Redefinition of parameter '" << node.ident << "'" << std::endl;
    }

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
    auto lvalNode = dynamic_cast<ast::LVal*>(node.lval.get());
    if (!lvalNode) {
        std::cerr << "Error: left-hand side of assignment is not an lvalue" << std::endl;
        return undefinedValue;
    }
    auto lval = getLValPointer(*lvalNode);
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
    if (!currentFunction) {
        std::cerr << "Error: return statement outside function" << std::endl;
        return nullptr;
    }

    if (node.exp && *node.exp) {
        auto retVal = (*node.exp)->accept(*this);
        return builder.CreateRet(retVal, currentFunction);
    } else {
        if (!currentFunction->getReturnType()->isVoidTy()) {
            std::cerr << "Error: Non-void function " << currentFunction->getIRName()
                      << " should return a value" << std::endl;
            return nullptr;
        }
        return builder.CreateRetVoid(currentFunction);
    }
}

IR::Value* Visitor::visit(const ast::Stmt::IfStmt &node) {
    auto trueBB = new IR::BasicBlock("if.true");
    auto falseBB = new IR::BasicBlock("if.false"); 
    auto mergeBB = new IR::BasicBlock("if.merge");

    if (currentFunction) {
        currentFunction->addBlock(trueBB);
        currentFunction->addBlock(falseBB);
        currentFunction->addBlock(mergeBB);
    }

    emitConditionalBranch(*node.exp, trueBB, falseBB);
    
    // 处理true分支
    builder.SetInsertPoint(trueBB);
    node.block->accept(*this);
    if (!currentBlockHasTerminator()) {
        builder.CreateBr(mergeBB);  
    }
    
    // 处理false分支
    if (node.elseStmt) {
        builder.SetInsertPoint(falseBB);
        (*node.elseStmt)->accept(*this);
        if (!currentBlockHasTerminator()) {
            builder.CreateBr(mergeBB);
        }
        
    } else {
        builder.SetInsertPoint(falseBB);
        builder.CreateBr(mergeBB);  
    }
    
    builder.SetInsertPoint(mergeBB);
    
    return nullptr;
}

IR::Value* Visitor::visit(const ast::Stmt::WhileStmt &node) {
    auto condBB = new IR::BasicBlock("while.cond");
    auto bodyBB = new IR::BasicBlock("while.body");
    loop_info loop;
    loop.continue_b = condBB;
    loop.break_b = new IR::BasicBlock("while.end");
    
    if (currentFunction) {
        currentFunction->addBlock(condBB);
        currentFunction->addBlock(bodyBB);
        currentFunction->addBlock(loop.break_b);
    }

    auto prevBB = builder.GetInsertBlock();
    
    builder.CreateBr(condBB);
    
    builder.SetInsertPoint(condBB);
    emitConditionalBranch(*node.cond, bodyBB, loop.break_b);

    builder.SetInsertPoint(bodyBB);
    loopStack.push(loop);
    node.stmt->accept(*this);
    loopStack.pop();
    
    if (!currentBlockHasTerminator()) {
        builder.CreateBr(condBB);
    }
    
    builder.SetInsertPoint(loop.break_b);

    
    return nullptr;
}

IR::Value* Visitor::visit(const ast::Stmt::BreakStmt &node) {
    if (!loopStack.empty()) {
        loop_info &loop = loopStack.top();
        return builder.CreateBr(loop.break_b);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::LVal &node) {
    auto symbol = symbolTable.lookup(node.ident);
    if (!symbol) {
        std::cerr << "Error: Undefined variable '" << node.ident << "'" << std::endl;
        return undefinedValue;
    }
    
    // 如果是常量，直接返回其�?
    if (symbol->isConst) {
        if (auto constant = dynamic_cast<IR::Constant*>(symbol->value)) {
            return constant;
        }
    }
    
    // 否则正常处理变量加载
    auto varPtr = getLValPointer(node);
    IR::pType varType = getTypeFromBType(symbol->baseType);
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
    auto left = coerceToBoolValue(node.lAndExp->accept(*this));
    auto right = coerceToBoolValue(node.eqExp->accept(*this));
    return builder.CreateAnd(left, right);
}

IR::Value* Visitor::visit(const ast::LOrExp &node) {
    return node.lOrExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::LOrExp::LOrExpOp& node) {
    auto left = coerceToBoolValue(node.lOrExp->accept(*this));
    auto right = coerceToBoolValue(node.lAndExp->accept(*this));
    return builder.CreateOr(left, right);
}

IR::Value* Visitor::visit(const ast::ConstExp& node) {
    return node.addExp->accept(*this);
}

IR::Value* Visitor::visit(const ast::Cond& node) {
    return node.lOrExp->accept(*this);
}

} // namespace frontend::visitor
