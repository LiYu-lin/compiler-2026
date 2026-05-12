#pragma once
#include "BasicOperand.h"
#include "OpRegister.h"
#include "OpImmediate.h"
#include <variant>
#include <memory>

namespace backend {

// 定义操作数变体类型（排除Immediate作为基址的可能性）
using AddrOperandVariant = std::variant<
    std::shared_ptr<VirtualRegister>,
    std::shared_ptr<PhysicalRegister>
>;

class Address : public Operand {
private:
    AddrOperandVariant base;
    std::shared_ptr<Immediate> offset;

    static OpType getMemTypeForReg(const AddrOperandVariant& reg) {
        if (std::holds_alternative<std::shared_ptr<VirtualRegister>>(reg)) {
            return std::get<std::shared_ptr<VirtualRegister>>(reg)->isFloatReg() 
                ? OpType::FloatMemory : OpType::IntMemory;
        } else {
            return std::get<std::shared_ptr<PhysicalRegister>>(reg)->isFloatReg()
                ? OpType::FloatMemory : OpType::IntMemory;
        }
    }

public:
    // 构造函�?
    Address(AddrOperandVariant base, std::shared_ptr<Immediate> offset)
        : Operand(getMemTypeForReg(base)), 
          base(std::move(base)), 
          offset(std::move(offset)) {}

    const AddrOperandVariant& getBase() const { return base; }

    std::shared_ptr<Immediate> getOffset() const { return offset; }

    bool isFloatMemory() const {
        return getType() == OpType::FloatMemory;
    }

    std::string toString() const override {
        std::string baseStr = std::visit([](auto&& arg) {
            return arg->toString();
        }, base);
        
        return offset->toString() + "(" + baseStr + ")";
    }

    std::string toString(const std::string& baseRegOverride = "") const {
        std::string baseStr;
        if (!baseRegOverride.empty()) {
            baseStr = baseRegOverride;
        } else {
            baseStr = std::visit([](auto&& arg) { return arg->toString(); }, base);
        }
        return offset->toString() + "(" + baseStr + ")";
    }
    
    // 设置基址寄存�?
    void setBase(const AddrOperandVariant& newBase) {
        base = newBase;
        type = getMemTypeForReg(newBase); // 更新类型
    }

    // 设置偏移�?
    void setOffset(std::shared_ptr<Immediate> newOffset) {
        offset = std::move(newOffset);
    }
};

} // namespace backend