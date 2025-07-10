#pragma once
#include "type.h"
#include "use.h"
#include "list.h"
#include <cassert> // 包含 assert 所需头文件
#include <iostream>
#define HANDLE_TYPECHEK_CREATE(value)               \
    bool is##value() const                          \
    {                                               \
        return getValueID() == ValueTy::value##Val; \
    }

namespace IR
{
    struct Constant;
    struct Value
    {
        enum ValueTy
        {
            // Constant
            TemporaryVal,
            FunctionVal,
            GlobalVariableVal,
            ConstantInt32Val,
            ConstantFloatVal,
            ConstantArrayVal,
            ConstantPointerVal,

            ArgumentVal,
            BasicBlockVal,

            InstructionVal,
        };

        virtual ~Value() = default;

        IR::pType type = nullptr; 

        List<Use *> useList;

        std::string name = "";

        int number = 0;

        const unsigned int subClassID = 0;

        std::vector<Use *> getVectorUses();

        Value(IR::pType type, const unsigned int subID) : type(type), subClassID(subID)
        {
            number = 0;
        };

        Value(std::string n, const unsigned int subID) : name(n), subClassID(subID)
        {
            number = 0;
        };


        Value(IR::pType ty, std::string n, const unsigned int subID) : type(ty), name(n), subClassID(subID)
        {
            number = 0;
        };

        void addUsage(Use *use);

        std::vector<User *> getUsers();

        virtual void emitUse(std::ostream &os);

        virtual void setInitializer(Constant *init);


        void setType(IR::pType type) { this->type = type; }


        IR::pType getType() const { return type; }

        std::string getTypeName() const { 
            if (type) {
                return type->to_string(); 
            }
            return "unknown";
        }

        std::string getOperandName() const { return "@" + name; }

        virtual std::string getIRName() const { return "@" + name; }

        virtual void emitIR(std::ostream &os) { os << getIRName() << std::endl; }

        virtual void waste();

        static int totalNumber;

        static void resetTotalNumber() { totalNumber = 0; }

        static Value* createTemp(IR::pType type);


        void replaceAllUsageTo(Value *newUser);

        unsigned int getValueID() const { return subClassID; }

        HANDLE_TYPECHEK_CREATE(Temporary)
        HANDLE_TYPECHEK_CREATE(Function)
        HANDLE_TYPECHEK_CREATE(GlobalVariable)
        HANDLE_TYPECHEK_CREATE(ConstantFloat)
        HANDLE_TYPECHEK_CREATE(ConstantInt32)
        HANDLE_TYPECHEK_CREATE(ConstantArray)
        HANDLE_TYPECHEK_CREATE(ConstantPointer)
        HANDLE_TYPECHEK_CREATE(Argument)
        HANDLE_TYPECHEK_CREATE(BasicBlock)
        bool isInstruction() const
        {
            return getValueID() >= ValueTy::InstructionVal;
        }
    };
}

#undef HANDLE_TYPECHEK_CREATE