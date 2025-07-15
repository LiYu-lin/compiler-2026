#pragma once

#include "Value/globalvalue.h"

namespace IR
{
    struct Module
    {
        std::vector<GlobalVariable *> globalVariableList;
        std::vector<Function *> functionList;
        std::map<std::string, Function *> builtinFunctions;
        std::string name;
        BasicBlock* globalInitBlock = nullptr;
        Module(std::string name);

        void addGlobal(GlobalValue *global)
        {
            if (global->isGlobalVariable())
                globalVariableList.push_back(static_cast<GlobalVariable *>(global));
            else if (global->isFunction())
                functionList.push_back(static_cast<Function *>(global));
        }
        void addGlobalInitBlock(BasicBlock* bb) {
            globalInitBlock = bb;
        }
        std::vector<GlobalVariable *> getGlobalVariableList()
        {
            return globalVariableList;
        }

        std::vector<Function *> getFunctionList()
        {
            return functionList;
        }
        bool hasGlobalInit() const {
            return globalInitBlock != nullptr;
        }
        void gen(std::ostream &os);

        void emitUse(std::ostream &os);

        void addBuiltinFunction(std::string name, ExternalFunction *func);

        Function *getBuiltinFunction(std::string name);
    };
}