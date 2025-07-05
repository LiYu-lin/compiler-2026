#pragma once
#include "mir/Value.h"
namespace IR
{
    struct User
        : public Value
    {
        std::vector<Value *> operands;
        std::vector<Use *> uses; 

        User(IR::pType type, const unsigned int subID) : Value(type, subID) {}
        User(IR::pType type, std::string name, const unsigned int subID) : Value(type, name, subID) {}

        virtual void addUse(Value *val);

        size_t getNumbOperands() const { return operands.size(); }
        virtual Value *getOperand(unsigned int i) const = 0;
        std::vector<Value *> &getOperands() { return operands; }

        virtual void removeUseFromVector(Use *use);

        static int totalUsers;

        void setTotalUsers() { number = totalUsers++; }
    };
}