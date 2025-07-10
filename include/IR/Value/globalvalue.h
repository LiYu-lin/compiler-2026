#pragma once

#include "constant.h"
#include <set>

namespace IR
{
    struct GlobalValue : public Constant
    {
        GlobalValue(pType type, const unsigned int ID) : Constant(type, ID) {}
        GlobalValue(pType type, std::string name, const unsigned int ID) : Constant(type, name, ID) {}
    };

    struct Function;
    struct Argument : public Value
    {
        Function *parent;
        Argument(pType type, std::string name) : Value(type, name, ValueTy::ArgumentVal) {}
        Argument(pType type, int argNum) : Value(type, ValueTy::ArgumentVal)
        {
            name = "arg" + std::to_string(argNum);
        }
    };
    struct Function : public GlobalValue
    {
        BasicBlock *entryBlock = nullptr;
        bool isBuiltin = false;
        std::vector<Argument *> funArgs;
        List<BasicBlock *> funBlocks;
        std::map<BasicBlock *, std::set<BasicBlock *>> cfg();

        Function(pType retType, std::string name);

        Function(pType retType, std::string name, std::vector<pType > argtypes);

        void waste() override;

        pType getReturnType() { return type->getPointerBase(); }

        List<BasicBlock *> &blocks() { return funBlocks; }
        std::vector<BasicBlock *> getVectorBlocks();

        std::vector<Argument *> &args() { return funArgs; }

        Argument *getArg(unsigned int i) { return funArgs[i]; }

       pType getArgType(unsigned int i) { return static_cast<const FunctionType *>(type)->getFunctionParam(i); }

        void addArgument(pType arg)
        {
            int id = funArgs.size();
            funArgs.push_back(new Argument(arg, id));
        }

        void addBlock(BasicBlock *block, bool entry = false);

        void insertBlock(BasicBlock *block, BasicBlock *prev);

        void removeBlock(BasicBlock *block);

        static Function *create(pType retType, std::vector<pType > argtypes, std::string name)
        {
            return new Function(retType, name, argtypes);
        }

        static Function *create(pType retType, std::string name)
        {
            return new Function(retType, name);
        }

        void emitIR(std::ostream &os) override;

        void emitUse(std::ostream &os) override;

        bool isBuiltinFunction() { return isBuiltin; }

        BasicBlock *getEntryBlock() { return entryBlock; }

        void mergeBlock(BasicBlock *prev, BasicBlock *nxt);

        BasicBlock *splitBlock(BasicBlock *block, Instruction *pos);
    };

    struct ExternalFunction : public Function
    {
        ExternalFunction(pType retType, std::string name, std::vector<pType > argtypes);

        void emitIR(std::ostream &os) override;

        static ExternalFunction *create(pType retType, std::vector<pType > argtypes, std::string name)
        {
            return new ExternalFunction(retType, name, argtypes);
        }
    };


    struct GlobalVariable final : public GlobalValue
    {
        bool isInit = false;
        bool isConstant = false;
        Constant *initializer;
        GlobalVariable(pType type, std::string name, bool isConstant = false);

        GlobalVariable(pType type, std::string name, Constant *initializer, bool isConstant = false);

        bool isInitialized() { return isInit; }
        bool isPointingConst() { return isConstant; }

        void emitIR(std::ostream &os) override;

        void setInitializer(Constant *init) override
        {
            initializer = init;
            isInit = true;
            operands[0] = init;
        }

        static GlobalVariable *create(pType type, std::string name, Constant *initializer, bool isConstant = false)
        {
            auto pointTy = PointerType::getPointerType(type);
            return new GlobalVariable(pointTy, name, initializer, isConstant);
        }

        static GlobalVariable *create(pType type, std::string name, bool isConstant = false)
        {
            auto pointTy = PointerType::getPointerType(type);
            return new GlobalVariable(pointTy, name, isConstant);
        }
    };
}