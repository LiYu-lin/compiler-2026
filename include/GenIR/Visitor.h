/**
 * @file SymbolTable.h
 * @author CoffeeRain
 * @brief Header file for the SymbolTable class.
 * @version 0.1
 * @date 2025-05-29
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include "ir/IRBuilder.h"
#include "Parse/AST.h"
#include "Parse/Lexer.h"
#include "ir/module.h"
#include "ir/type.h"
#include "ir/value.h"
#include <ostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <stack>
namespace frontend::visitor {

using FuncType = ast::ASTNode::FuncType;

struct SymbolInfo {
    std::string name;
    ast::ASTNode::BType baseType;
    FuncType funcType;
    bool isConst;
    bool isArray;
    int scopeLevel;
    std::vector<int> dims;
    IR::Value* value;

    // 添加默认构造函数
    SymbolInfo() 
        : name(""), 
          baseType(ast::ASTNode::BType::Int),
          funcType(FuncType::Void),
          isConst(false),
          isArray(false),
          scopeLevel(0),
          value(nullptr) {}
    bool isFunction = false;
    // 保持原有构造函数不变
    SymbolInfo(std::string name, ast::ASTNode::BType type, bool isConst, int scopeLevel)
        : name(name), baseType(type), isConst(isConst), scopeLevel(scopeLevel), 
          isArray(false), funcType(FuncType::Void), value(nullptr) {}


    
    SymbolInfo(std::string name, FuncType type, int scopeLevel)
        : name(name), funcType(type), scopeLevel(scopeLevel),
          baseType(ast::ASTNode::BType::Int), isConst(false), 
          isArray(false), value(nullptr), isFunction(true) {}
};

// 然后保持insert函数中的isFunction判断不变

class SymbolTable {
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;
    int currentScopeLevel = -1;

public:

    SymbolTable() { enterScope(); } 

    int getCurrentScopeLevel() const { return currentScopeLevel; }

    void enterScope() {
        scopes.emplace_back();
        currentScopeLevel++;
    }

    void exitScope() {
        if (currentScopeLevel > -1) {
            scopes.pop_back();
            currentScopeLevel--;
        }
    }

    bool insert(const std::string& name, const SymbolInfo& info) {
        int param = 1;
        if(param)
        {
            std::cout<<"Name:"<<name<<", ";
            if (info.isFunction)
            {
                std::cout << "type: function, returnType: ";
                switch (info.funcType)
                {
                    case FuncType::Void: std::cout << "void"; break;
                    case FuncType::Int: std::cout << "int"; break;
                    case FuncType::Float: std::cout << "float"; break;
                }
            }
            else
            {
                std::cout << "type: " << (info.isConst ? "const variable" : "variable") << ", ";
                std::cout << "baseType: ";
                switch (info.baseType)
                {
                    case ast::ASTNode::BType::Int: std::cout << "int"; break;
                    case ast::ASTNode::BType::Float: std::cout << "float"; break;
                }
                std::cout << ", dims: [";
                for (size_t d = 0; d < info.dims.size(); ++d)
                {
                    std::cout << info.dims[d];
                    if (d + 1 < info.dims.size()) std::cout << ",";
                }
                std::cout << "]";
            }
            std::cout << ", scopeLevel: " << info.scopeLevel << "\n";
        }
        auto& current = scopes.back();
        if (current.count(name)) return false;
        current[name] = info;
        return true;
    }

    const SymbolInfo* lookup(const std::string& name) const {
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            if (auto found = it->find(name); found != it->end()) {
                return &found->second;
            }
        }
        return nullptr;
    }
};


class Visitor {
    IR::IRBuilder builder;
    ast::ASTNode::BType currentBType;
    //gzj: 遇到continue就进入continue_b，遇到break就进入break_b
    struct loop_info{
        IR::BasicBlock *continue_b = nullptr;
        IR::BasicBlock *break_b = nullptr;
    };
    std::stack<loop_info> loopStack;
    struct condition_info {
        IR::BasicBlock *true_b = nullptr;
        IR::BasicBlock *false_b = nullptr;
    };
    std::stack<condition_info> conditionStack;
    IR::Module &module;
    SymbolTable symbolTable;
    IR::pType currentType = IR::Type::getVoidType();
    IR::Function *currentFunction = nullptr;
    IR::Value *undefinedValue = new IR::Value(IR::Type::getVoidType(), "undefined", 0);
    IR::BasicBlock *currentBB = nullptr;
    frontend::TokenPtrs token_buffer;
    public:

    IR::pType getTypeFromBType(ast::ASTNode::BType btype);
    ast::ASTNode::BType getBTypeFromAST(ast::ASTNode::BType btype);
    IR::pType getTypeFromfuncType(const FuncType &funcType);

    IR::Value* visit(const ast::ASTNode &node);
    IR::Value* visit(const ast::CompUnit &node);
    IR::Value* visit(const ast::Decl &node);
    IR::Value* visit(const ast::ConstDecl &node);
    IR::Value* visit(const ast::ConstDef &node);
    IR::Value* visit(const ast::VarDecl &node);
    IR::Value* visit(const ast::VarDef &node);
    IR::Value* visit(const ast::InitVal &node);
    IR::Value* visit(const ast::FuncDef &node);
    IR::Value* visit(const ast::FuncFParam &node);
    IR::Value* visit(const ast::FuncFParams &node);
    IR::Value* visit(const ast::Block &node);
    IR::Value* visit(const ast::BlockItem &node);
    IR::Value* visit(const ast::Stmt &node);
    IR::Value* visit(const ast::Stmt::AssignStmt &node);
    IR::Value* visit(const ast::Stmt::ExpStmt &node);
    IR::Value* visit(const ast::Stmt::BlockStmt &node);
    IR::Value* visit(const ast::Stmt::IfStmt &node);
    IR::Value* visit(const ast::Stmt::WhileStmt &node);
    IR::Value* visit(const ast::Stmt::BreakStmt &node);
    IR::Value* visit(const ast::Stmt::ContinueStmt &node);
    IR::Value* visit(const ast::Stmt::ReturnStmt &node);
    IR::Value* visit(const ast::LVal &node);
    IR::Value* visit(const ast::Number &node);
    IR::Value* visit(const ast::Cond &node);
    IR::Value* visit(const ast::Exp &node);
    IR::Value* visit(const ast::PrimaryExp &node);
    IR::Value* visit(const ast::UnaryExp &node);
    IR::Value* visit(const ast::UnaryExp::UnaryExpCall &node);
    IR::Value* visit(const ast::UnaryExp::UnaryExpOp &node);
    IR::Value* visit(const ast::FuncRParams &node);
    IR::Value* visit(const ast::ConstInitVal& node);
    IR::Value* visit(const ast::MulExp &node);
    IR::Value* visit(const ast::MulExp::MulExpOp &node);
    IR::Value* visit(const ast::AddExp &node);
    IR::Value* visit(const ast::AddExp::AddExpOp &node);
    IR::Value* visit(const ast::RelExp &node);
    IR::Value* visit(const ast::RelExp::RelExpOp &node);
    IR::Value* visit(const ast::EqExp &node);
    IR::Value* visit(const ast::EqExp::EqExpOp &node);
    IR::Value* visit(const ast::LAndExp &node);
    IR::Value* visit(const ast::LAndExp::LAndExpOp &node);
    IR::Value* visit(const ast::LOrExp &node);
    IR::Value* visit(const ast::LOrExp::LOrExpOp &node);
    IR::Value* visit(const ast::ConstExp &node);
    
    Visitor(IR::Module &module) : module(module), builder() {}
};

} 