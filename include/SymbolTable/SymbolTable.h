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
#include <ostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
namespace frontend {

enum class BType { Int, Float };
enum class FuncType { Void, Int, Float };

struct SymbolInfo {
    std::string name;
    bool isConst;          
    bool isFunction;       
    BType baseType;        
    FuncType funcType;     
    std::vector<int> dims; 
    int scopeLevel;        

    SymbolInfo() = default;
    
    SymbolInfo(std::string name, BType type, bool isConst, int scopeLevel)
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
                    case BType::Int: std::cout << "int"; break;
                    case BType::Float: std::cout << "float"; break;
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

} 