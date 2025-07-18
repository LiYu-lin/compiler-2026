#include "GenIR/Visitor.h"
#include "Parse/AST.h"
#include "ir/Value/globalvalue.h"
#include "ir/basicblock.h"
#include "ir/type.h"
#include "ir/value.h"
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
    // 添加调试输出，帮助定位问题
    std::cout << "Debug: getTypeFromBType called with btype=" 
              << static_cast<int>(btype) << std::endl;
              
    switch (btype) {
        case BType::Int: 
            std::cout << "Debug: Returning i32 type" << std::endl;
            return IR::Type::getI32Type();
        case BType::Float: 
            std::cout << "Debug: Returning float type" << std::endl;
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
    std::cout <<"visit node begin" << std::endl;
    return const_cast<ast::ASTNode&>(node).accept(*this);
}

IR::Value* Visitor::visit(const ast::CompUnit &node) {
    std::cout << "Visiting CompUnit with " << node.decls.size() << " declarations." << std::endl;
    auto globalInitBB = new IR::BasicBlock("global.init");
    builder.SetInsertPoint(globalInitBB);
    for (const auto &decl : node.decls) {
        if (!decl) {
            std::cerr << "Warning: null declaration in CompUnit" << std::endl;
            continue;
        }
        std::cout << "Visiting declaration: "<< std::endl;
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
    std::cout << "Visiting Decl " << std::endl;
    return node.decl->accept(*this);
}

IR::Value* Visitor::visit(const ast::ConstDecl &node) {
    std::cout << "Visiting ConstDecl with btype=" << static_cast<int>(node.btype) << std::endl;
    currentBType = node.btype;
    for (const auto &def : node.constDefs) {
        def->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::InitVal& node) {
    std::cout << "Visiting InitVal." << std::endl;
    if (std::holds_alternative<ast::ASTNodePtr>(node.initVal)) {
        std::cout << "InitVal contains ASTNodePtr." << std::endl;
        return std::get<ast::ASTNodePtr>(node.initVal)->accept(*this);
    } else {
        std::cout << "InitVal contains unexpected type." << std::endl;
        return undefinedValue;
    }
}

IR::Value* Visitor::visit(const ast::ConstDef& node) {
    std::cout << "Visiting ConstDef: " << node.ident << std::endl;
    
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
        constValue = new IR::GlobalVariable(constType, node.ident, initVal, true); // true表示是常量
        module.addGlobal(static_cast<IR::GlobalVariable*>(constValue));
    } else {  // 局部常量处理
        constValue = builder.CreateAlloca(constType, node.ident, true);
        if (node.constInitVal) {
            auto initVal = node.constInitVal->accept(*this);
            builder.CreateStore(initVal, constValue);
        }
    }

    // 设置符号表信息
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
    std::cout << "Visiting ConstInitVal." << std::endl;
    if (std::holds_alternative<ast::ASTNodePtr>(node.constInitVal)) {
        auto ptr = std::get<ast::ASTNodePtr>(node.constInitVal);
        std::cout << "ConstInitVal contains ASTNodePtr: " << ptr->toString(0) << std::endl;
        auto val = ptr->accept(*this);
        if (!dynamic_cast<IR::Constant*>(val)) {
            std::cerr << "Error: ConstInitVal must evaluate to constant" << std::endl;
            return undefinedValue;
        }
        return val;
    } else {
        std::cout << "ConstInitVal contains unexpected type." << std::endl;
        return undefinedValue;
    }
}

IR::Value* Visitor::visit(const ast::Number &node) {
    if (std::holds_alternative<int>(node.value)) {
        int val = std::get<int>(node.value);
        std::cout << "Visiting Number with int value: " << val << std::endl;
        return IR::ConstantInt32::get(val);
    } else {
        float val = std::get<float>(node.value);
        std::cout << "Visiting Number with float value: " << val << std::endl;
        return IR::ConstantFloat::get(val);
    }
}
IR::Value* Visitor::visit(const ast::VarDecl &node) {
    std::cout<< "Visiting VarDecl with btype=" << static_cast<int>(node.btype) << std::endl;
    currentBType = node.btype;
    for (const auto &def : node.varDefs) {
        def->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::VarDef &node) {
    std::cout << "Visiting VarDef: " << node.ident << std::endl;
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
    
    // 区分全局变量和局部变量
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

    // 符号表记录
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
    std::cout<< "Visiting FuncDef: " << node.ident << std::endl<< std::flush;
    
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
    auto entryBB = new IR::BasicBlock("entry");
    currentFunction->addBlock(entryBB,true);
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
    std::cout << "Visiting FuncFParams with " << node.funcFParams.size() << " parameters." << std::endl;
    for (const auto& param : node.funcFParams) {
        param->accept(*this);
    }
    return nullptr;
}

IR::Value* Visitor::visit(const ast::FuncFParam& node) {
    std::cout << "Visiting FuncFParam: " << node.ident << std::endl;   

    IR::pType paramType = getTypeFromBType(node.btype);
    
    if (node.dimensions && !node.dimensions->empty()) {
        paramType = IR::ArrayType::getArrayType(node.dimensions->size(), paramType);
    }
    
    auto alloca = builder.CreateAlloca(paramType, node.ident);
    return alloca;
}

IR::Value* Visitor::visit(const ast::Block &node) {
    std::cout << "Visiting Block with " << node.blockItems.size() << " items." << std::endl;
    symbolTable.enterScope();
    for (const auto &item : node.blockItems) {
        item->accept(*this);
    }
    symbolTable.exitScope();
    return nullptr;
}

IR::Value* Visitor::visit(const ast::BlockItem &node) {
    std::cout << "Visiting BlockItem." << std::endl;
    return node.item->accept(*this);
}

IR::Value* Visitor::visit(const ast::Stmt &node) {
    std::cout << "Visiting Stmt." << std::endl;
    return node.stmt->accept(*this);
}

IR::Value* Visitor::visit(const ast::Stmt::AssignStmt& node) {
    std::cout << "Visiting AssignStmt." << std::endl;
    auto lval = node.lval->accept(*this);
    auto exp = node.exp->accept(*this);
    builder.CreateStore(exp, lval);
    return exp;
}

IR::Value* Visitor::visit(const ast::Stmt::ExpStmt& node) {
    std::cout << "Visiting ExpStmt." << std::endl;

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
        std::cout << "Returning value from function: " << currentFunction->getIRName() << std::endl;
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
    auto cond = node.exp->accept(*this);
    auto trueBB = new IR::BasicBlock("if.true");
    auto falseBB = new IR::BasicBlock("if.false");
    auto mergeBB = new IR::BasicBlock("if.merge");

    if (currentFunction) {
        currentFunction->addBlock(trueBB);
        currentFunction->addBlock(falseBB);
        currentFunction->addBlock(mergeBB);
    }

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

    if (currentFunction) {
        currentFunction->addBlock(loop.continue_b);
        currentFunction->addBlock(loop.break_b);
    }   

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

IR::Value* Visitor::visit(const ast::LVal &node) {
    auto symbol = symbolTable.lookup(node.ident);
    if (!symbol) {
        std::cerr << "Error: Undefined variable '" << node.ident << "'" << std::endl;
        return undefinedValue;
    }
    
    // 如果是常量，直接返回其值
    if (symbol->isConst) {
        if (auto constant = dynamic_cast<IR::Constant*>(symbol->value)) {
            return constant;
        }
    }
    
    // 否则正常处理变量加载
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

IR::Value* Visitor::visit(const ast::Cond& node) {
    return node.lOrExp->accept(*this);
}

} // namespace frontend::visitor