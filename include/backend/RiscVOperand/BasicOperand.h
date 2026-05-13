#pragma once
#include <string>

namespace backend {

class Operand {
public:
    enum class OpType {
        Void = 0, 
        IntImm,   // 整数立即�?
        FloatImm, // 浮点立即�?
        IntReg,   // 整数寄存�?
        FloatReg, // 浮点寄存�?
        IntMemory, // 整数内存地址
        FloatMemory, // 浮点内存地址
        Stack,    // 栈位�?
        Label     // 标签
    };


    OpType type;

public:
    explicit Operand(OpType type) : type(type) {}
    virtual ~Operand() = default;

    OpType getType() const { return type; }
    
    std::string getTypeName() const {
        switch (type) {
            case OpType::Void: return "void";
            case OpType::IntImm: return "int_imm";
            case OpType::FloatImm: return "float_imm";
            case OpType::IntReg: return "int_reg";
            case OpType::FloatReg: return "float_reg";
            case OpType::IntMemory: return "int_mem";
            case OpType::FloatMemory: return "float_mem";
            case OpType::Stack: return "stack";
            case OpType::Label: return "label";
            default: return "unknown";
        }
    }

    virtual std::string toString() const = 0;
};

} // namespace backend
