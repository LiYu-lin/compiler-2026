#pragma once
#include "BasicOperand.h"
#include "OpAddress.h"
#include <memory>
#include <string>

namespace backend {

class StackVariable : public Operand {
public:
    enum class VarType {
        Int32,
        Float32,
        Pointer,
        Array,
        FloatArray // 新增：显式支持浮点数数组
    };

    StackVariable(VarType type, std::shared_ptr<Address> addr, 
                  bool useFP = true, bool preserved = false)
        : Operand((type == VarType::Float32 || type == VarType::FloatArray) ? OpType::FloatMemory : OpType::IntMemory),
          varType(type),
          address(std::move(addr)),
          useFramePointer(useFP),
          isRegPreserved(preserved) {
        initStandardSize();
    }

    // 自定义多维数数组安全构造函数
    StackVariable(int customSize, int align, std::shared_ptr<Address> addr, bool isFloatArray = false,
                  bool useFP = true, bool preserved = false)
        : Operand(isFloatArray ? OpType::FloatMemory : OpType::IntMemory),  
          varType(isFloatArray ? VarType::FloatArray : VarType::Array),
          address(std::move(addr)),
          size(customSize),
          alignment(align),
          useFramePointer(useFP),
          isRegPreserved(preserved) {}

    VarType getVarType() const { return varType; }

    std::string getAddressing() const {
        const std::string& baseReg = useFramePointer ? "s0" : "sp";
        return address->toString(baseReg);
    }

    std::string toString() const override {
        std::string typeStr;
        switch(varType) {
            case VarType::Int32:      typeStr = "i32"; break;
            case VarType::Float32:    typeStr = "f32"; break;
            case VarType::Pointer:    typeStr = "ptr"; break;
            case VarType::Array:      typeStr = "arr_i"; break;
            case VarType::FloatArray: typeStr = "arr_f"; break;
        }
        return typeStr + "[" + std::to_string(size) + "]@" + getAddressing();
    }

    int getSize() const { return size; }
    int getAlignment() const { return alignment; }
    bool usesFramePointer() const { return useFramePointer; }
    bool needsRegisterPreservation() const { return isRegPreserved; }

    std::shared_ptr<Address> getAddress() const { return address; }
    void setAddress(std::shared_ptr<Address> newAddr) { 
        address = std::move(newAddr); 
        updateOperandType();
    }

    bool isRegister() const { return false; }
    bool isMemory() const { return true; }
    bool isImmediate() const { return false; }

private:
    void initStandardSize() {
        switch(varType) {
            case VarType::Int32:       size = 4; alignment = 4; break;
            case VarType::Float32:     size = 4; alignment = 4; break;
            case VarType::Pointer:     size = 8; alignment = 8; break;
            case VarType::Array:       size = 0; alignment = 4; break; 
            case VarType::FloatArray:  size = 0; alignment = 4; break;
        }
    }

    void updateOperandType() {
        if (varType == VarType::Float32 || varType == VarType::FloatArray) {
            type = OpType::FloatMemory;
        } else {
            type = OpType::IntMemory;
        }
    }

    VarType varType;
    std::shared_ptr<Address> address;
    int size;
    int alignment;
    bool useFramePointer;
    bool isRegPreserved;
};

} // namespace backend