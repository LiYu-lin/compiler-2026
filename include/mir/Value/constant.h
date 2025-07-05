#pragma once

#include "user.h"
#include "mir/type.h"
#include <cassert>
#include <map>
#include <stdexcept>
#include <string>

namespace IR
{
    inline void error(const char* func, const std::string& msg) {
        throw std::runtime_error(std::string(func) + ": " + msg);
    }

    struct Constant : public User
    {
        Constant(pType type, const unsigned int subID) : User(type, subID) {}

        Constant(pType type, std::string name, const unsigned int subID) : User(type, name, subID) {}

        Constant *getOperand(unsigned int i) const override
        {
            if (i >= operands.size())
                error(__PRETTY_FUNCTION__, "Index out of bounds");
            return static_cast<Constant *>(operands[i]);
        };

        static Constant *getZeroValueForType(pType type);
        // 类型判断接口
        virtual bool isConstantArray() const { return false; }
    };

    struct ConstantInt32 final : public Constant
    {
        int value;

        ConstantInt32(int v) : Constant(Type::getI32Type(), ValueTy::ConstantInt32Val), value(v) {}
        ConstantInt32(int v, std::string name) : Constant(Type::getI32Type(), name, ValueTy::ConstantInt32Val), value(v) {}
        int getValue() const { return value; }

        static ConstantInt32 *get(int value)
        {
            return new ConstantInt32(value);
        }

        std::string getIRName() const override
        {
            return std::to_string(value);
        }
    };

    struct ConstantFloat final : public Constant
    {
        float value;

        ConstantFloat(float v) : Constant(Type::getFloatType(), ValueTy::ConstantFloatVal), value(v) {}
        ConstantFloat(float v, std::string name) : Constant(Type::getFloatType(), name, ValueTy::ConstantFloatVal), value(v) {}
        float getValue() const { return value; }

        static ConstantFloat *get(float value)
        {
            return new ConstantFloat(value);
        }

        std::string getIRName() const override
        {
            return std::to_string(value);
        }
    };

    struct ConstantArray : public Constant
    {
        std::map<unsigned int, Constant *> elements;

        ConstantArray(pType type) : Constant(type, ValueTy::ConstantArrayVal) {}
        ConstantArray(pType type, std::string name) : Constant(type, name, ValueTy::ConstantArrayVal) {}

        Constant *getOperand(unsigned int i) const override
        {
            auto element = elements.find(i);
            if (element == elements.end())
            {
                // type 必须是 ArrayType
                auto arrType = static_cast<const ArrayType*>(type);
                if ((int)i >= arrType->getArraySize())
                    error(__PRETTY_FUNCTION__, "Index out of bounds");
                return Constant::getZeroValueForType(arrType->getArrayBase());
            }
            return element->second;
        }

        void insertOperand(unsigned int i, Constant *value)
        {
            elements[i] = value;
        }

        static ConstantArray *get(pType type)
        {
            if (!type->isArrayTy())
                error(__PRETTY_FUNCTION__, "Type is not an array type");
            return new ConstantArray(type);
        }

        Constant *addIndex(pType ty, unsigned int i, Constant *value)
        {
            if (elements.find(i) == elements.end())
            {
                insertOperand(i, value);
            }
            else
            {
                error(__PRETTY_FUNCTION__, "Index has been set");
            }
            return elements[i];
        }

        ConstantArray *addIndex(pType ty, unsigned int i)
        {
            if (elements.find(i) == elements.end())
            {
                elements[i] = ConstantArray::get(ty);
            }
            assert(elements[i]->isConstantArray());
            return static_cast<ConstantArray *>(elements[i]);
        }

        bool isConstantArray() const override { return true; }
    };

    // Constant Pointer points to a constantInt32 or a constantFloat
    struct ConstantPointer : public Constant
    {
        Constant *Value;

        ConstantPointer(Constant *value)
            : Constant(Type::getPointerType(value->getType()), ValueTy::ConstantPointerVal)
        {
            operands.push_back(value);
        }
        ConstantPointer(std::string name, Constant *value)
            : Constant(Type::getPointerType(value->getType()), name, ValueTy::ConstantPointerVal)
        {
            operands.push_back(value);
        }

        Constant *getOperand(unsigned int i) const override
        {
            if (i != 0)
                error(__PRETTY_FUNCTION__, "Index out of bounds");
            return static_cast<Constant *>(operands[0]);
        }

        static ConstantPointer *get(Constant *value)
        {
            return new ConstantPointer(value);
        }
    };
}