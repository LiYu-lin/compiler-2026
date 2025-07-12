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


enum class FuncType { Void, Int, Float };

struct SymbolInfo {
    std::string name;
    bool isConst;          
    bool isFunction;       
    ast::ASTNode::BType baseType;        
    FuncType funcType;     
    std::vector<int> dims; 
    int scopeLevel;        
    IR::Value* value = nullptr; 
    SymbolInfo() = default;
    
    SymbolInfo(std::string name, ast::ASTNode::BType type, bool isConst, int scopeLevel)
        : name(name), isConst(isConst), isFunction(false),
          baseType(type), scopeLevel(scopeLevel) {}

    SymbolInfo(std::string name, FuncType type, int scopeLevel)
        : name(name),funcType(type),
          scopeLevel(scopeLevel) {
            isFunction= true;
          }
};

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

    IR::Value* visit(const ast::ASTNode &node);
    
    IR::Value* visit(ast::CompUnit &node);
    IR::Value* visit(ast::Decl &node);
    IR::Value* visit(ast::ConstDecl &node);
    IR::Value* visit(ast::ConstDef &node);
    IR::Value* visit(ast::VarDecl &node);
    IR::Value* visit(ast::VarDef &node);
    IR::Value* visit(ast::InitVal &node);

    IR::Value* visit(ast::FuncDef &node);
    IR::Value* visit(ast::FuncFParam &node);
    IR::Value* visit(ast::FuncFParams &node);

    IR::Value* visit(ast::Block &node);
    IR::Value* visit(ast::BlockItem &node);

    IR::Value* visit(ast::Stmt &node);
    IR::Value* visit(ast::Stmt::AssignStmt &node);
    IR::Value* visit(ast::Stmt::ExpStmt &node);
    IR::Value* visit(ast::Stmt::BlockStmt &node);
    IR::Value* visit(ast::Stmt::IfStmt &node);
    IR::Value* visit(ast::Stmt::WhileStmt &node);
    IR::Value* visit(ast::Stmt::BreakStmt &node);
    IR::Value* visit(ast::Stmt::ContinueStmt &node);
    IR::Value* visit(ast::Stmt::ReturnStmt &node);

    IR::Value* visit(ast::LVal &node);
    IR::Value* visit(ast::Number &node);
    IR::Value* visit(ast::Cond &node);

    IR::Value* visit(ast::Exp &node);
    IR::Value* visit(ast::PrimaryExp &node);
    IR::Value* visit(ast::UnaryExp &node);
    IR::Value* visit(ast::UnaryExp::UnaryExpCall &node);
    IR::Value* visit(ast::UnaryExp::UnaryExpOp &node);

    IR::Value* visit(ast::FuncRParams &node);
    IR::Value* visit(ast::ConstInitVal& node);

    IR::Value* visit(ast::MulExp &node);
    IR::Value* visit(ast::MulExp::MulExpOp &node);

    IR::Value* visit(ast::AddExp &node);
    IR::Value* visit(ast::AddExp::AddExpOp &node);

    IR::Value* visit(ast::RelExp &node);
    IR::Value* visit(ast::RelExp::RelExpOp &node);

    IR::Value* visit(ast::EqExp &node);
    IR::Value* visit(ast::EqExp::EqExpOp &node);

    IR::Value* visit(ast::LAndExp &node);
    IR::Value* visit(ast::LAndExp::LAndExpOp &node);

    IR::Value* visit(ast::LOrExp &node);
    IR::Value* visit(ast::LOrExp::LOrExpOp &node);

    IR::Value* visit(ast::ConstExp &node);
    Visitor(IR::Module &module) : module(module), builder() {}


};

} 