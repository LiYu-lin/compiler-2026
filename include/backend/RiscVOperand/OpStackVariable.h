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
        Array
    };

    // 构造函数 - 标准类型
    StackVariable(VarType type, std::shared_ptr<Address> addr, 
                 bool useFP = true, bool preserved = false)
        : Operand(type == VarType::Float32 ? OpType::FloatMemory : OpType::IntMemory),
          varType(type),
          address(std::move(addr)),
          useFramePointer(useFP),
          isRegPreserved(preserved) {
        initStandardSize();
    }

    // 构造函数 - 自定义大小(数组等)
    StackVariable(int customSize, int align, std::shared_ptr<Address> addr,
                 bool useFP = true, bool preserved = false)
        : Operand(OpType::IntMemory),  // 数组默认按整型内存处理
          varType(VarType::Array),
          address(std::move(addr)),
          size(customSize),
          alignment(align),
          useFramePointer(useFP),
          isRegPreserved(preserved) {}

    // 获取变量类型
    VarType getVarType() const { return varType; }

    // 获取内存地址表示
    std::string getAddressing() const {
        const std::string& baseReg = useFramePointer ? "s0" : "sp";
        return address->toString(baseReg);
    }

    // 转换为字符串表示
    std::string toString() const override {
        std::string typeStr;
        switch(varType) {
            case VarType::Int32:   typeStr = "i32"; break;
            case VarType::Float32: typeStr = "f32"; break;
            case VarType::Pointer: typeStr = "ptr"; break;
            case VarType::Array:   typeStr = "arr"; break;
        }
        return typeStr + "[" + std::to_string(size) + "]@" + getAddressing();
    }

    // 内存属性访问
    int getSize() const { return size; }
    int getAlignment() const { return alignment; }
    bool usesFramePointer() const { return useFramePointer; }
    bool needsRegisterPreservation() const { return isRegPreserved; }

    // 地址操作
    std::shared_ptr<Address> getAddress() const { return address; }
    void setAddress(std::shared_ptr<Address> newAddr) { 
        address = std::move(newAddr); 
        updateOperandType();
    }

    // 操作数类型判断
    bool isRegister() const { return false; }
    bool isMemory() const{ return true; }
    bool isImmediate() const  { return false; }

private:
    void initStandardSize() {
        switch(varType) {
            case VarType::Int32:   size = 4; alignment = 4; break;
            case VarType::Float32: size = 4; alignment = 4; break;
            case VarType::Pointer: size = 8; alignment = 8; break;
            case VarType::Array:    size = 0; alignment = 4; break; // 数组大小由构造函数指定
        }
    }

    void updateOperandType() {
        if (varType == VarType::Float32) {
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