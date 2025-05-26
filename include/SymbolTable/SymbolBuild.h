// SymbolBuilder.h
#pragma once
//yellow warning need to be solved
#include "Parse/Parser.h"
#include "SymbolTable.h"
#include <ostream>

namespace frontend {

class SymbolBuilder {
    SymbolTable& symTable;
    BType currentBType;    // 当前处理的变量类型
    bool isConstDecl = false; // 是否在常量声明中

public:
    SymbolBuilder(SymbolTable& table) : symTable(table) {}

    void build(const ast::ASTNodePtr& node) {
        switch (node->type) {
        case ast::ASTNode::Type::CompUnit:
            handleCompUnit(static_cast<ast::CompUnit&>(*node));
            break;
        case ast::ASTNode::Type::Decl:
            handleDecl(static_cast<ast::Decl&>(*node));
            break;
        case ast::ASTNode::Type::ConstDecl:
            handleConstDecl(static_cast<ast::ConstDecl&>(*node));
            break;
        
        case ast::ASTNode::Type::ConstDef:
            handleConstDef(static_cast<ast::ConstDef&>(*node));
            break;

        case ast::ASTNode::Type::FuncDef:
            handleFuncDef(static_cast<ast::FuncDef&>(*node));
            break;
        case ast::ASTNode::Type::FuncFParams:
            std::cout<<"find funcfparams case"<<std::endl;
            handleFuncFParams(static_cast<ast::FuncFParams&>(*node));
            break;
        case ast::ASTNode::Type::FuncFParam:
            std::cout<<"find funcfparam case"<<std::endl;
            handleFuncFParam(static_cast<ast::FuncFParam&>(*node));
            break;
        case ast::ASTNode::Type::VarDecl:
            handleVarDecl(static_cast<ast::VarDecl&>(*node));
            break; 
        case ast::ASTNode::Type::VarDef:
            handleVarDef(static_cast<ast::VarDef&>(*node));
            break;
        case ast::ASTNode::Type::Block:
            handleBlock(static_cast<ast::Block&>(*node));
            break;
         default:
            break;
        //     throw ("未知节点类型: " + std::to_string(static_cast<int>(node->type)));
        }
 
    }

private:
    // 处理编译单元（全局声明）
    void handleCompUnit(const ast::CompUnit& compUnit) {
        for (const auto& decl : compUnit.decls) {
            build(decl);
        }
    }

    // 处理声明（常量/变量/函数）
    void handleDecl(const ast::Decl& decl) {
        build(decl.decl);
    }

    // 处理常量声明
    void handleConstDecl(const ast::ConstDecl& constDecl) {
        isConstDecl = true;
        currentBType = BType(constDecl.btype);
        for (const auto& def : constDecl.constDefs) {
            build(def);
        }
        isConstDecl = false;
    }

    // 处理常量定义
    void handleConstDef(const ast::ConstDef& constDef) {
        // 提取维度信息（数组）
        std::vector<int> dims;
        for (const auto& dim : constDef.dimensions) {
            // 假设维度是常量表达式（如ConstExp）
            // 实际实现需要计算常量表达式的值
            dims.push_back(0); // 示例用0占位
        }

        SymbolInfo info(
            constDef.ident,
            currentBType,
            true, // isConst
            symTable.getCurrentScopeLevel()
        );
        info.dims = dims;

        if (!symTable.insert(constDef.ident, info)) {
            throw ("重复定义常量: " + constDef.ident);
        }
    }

    // 处理函数定义
    void handleFuncDef(const ast::FuncDef& funcDef) {
        // 插入函数符号到当前作用域
        SymbolInfo funcInfo(
            funcDef.ident,
            FuncType(funcDef.funcType),//这里又构造functype是因为ast和symboltable的functype不一样
            symTable.getCurrentScopeLevel()
        );
        if (!symTable.insert(funcDef.ident, funcInfo)) {
            throw ("重复定义函数: " + funcDef.ident);
        }

        // 进入函数作用域
        symTable.enterScope();

        // 处理参数
        if(funcDef.funcFParams) {
            build(funcDef.funcFParams);
        }

        // 处理函数体
        build(funcDef.block);

        symTable.exitScope();
    }
    void handleFuncFParams(const ast::FuncFParams& funcFParams) {
        for (const auto& param : funcFParams.funcFParams) {
            build(param);
        }
    }
    void handleFuncFParam(const ast::FuncFParam& funcFParam) {
        // 处理函数参数

        SymbolInfo info(
            funcFParam.ident, // 需替换为实际获取逻辑
            BType(funcFParam.btype), 
            false, // 参数非常量
            symTable.getCurrentScopeLevel()
        );

        symTable.insert(info.name, info);
    }

void handleVarDecl(const ast::VarDecl& varDecl) {
    currentBType = BType(varDecl.btype);
    for (const auto& varDef : varDecl.varDefs) {
        build(varDef);
    }
}

void handleVarDef(const ast::VarDef& varDef) {
    std::vector<int> dims;
    for (const auto& dim : varDef.dimensions) {
        dims.push_back(0); // 示例占位
    }

    SymbolInfo info(
        varDef.ident,
        currentBType,
        false, // 变量
        symTable.getCurrentScopeLevel()
    );
    info.dims = dims;

    if (!symTable.insert(varDef.ident, info)) {
        throw ("重复定义变量: " + varDef.ident);
    }
}

void handleBlock(const ast::Block& block) {
    symTable.enterScope();
    for (const auto& item : block.blockItems) {
        build(item);
    }
    symTable.exitScope();
}

};

} // namespace frontend