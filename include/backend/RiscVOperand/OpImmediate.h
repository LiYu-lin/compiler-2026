#pragma once
#include "BasicOperand.h"  
#include <memory>
#include <utility>
#include <cassert>

namespace backend {

class Immediate : public Operand {
public:
    enum class ImmType {
        Int,
        Float,
        LabelOffset,  // 用于标签偏移
        HiLoSplit     // 用于高低位分裂
    };

    enum class SplitPart { HI20, LO12 };

    Immediate(size_t value)
        : Operand(OpType::IntImm),
        immType(ImmType::Int),
        intValue(static_cast<int32_t>(value)) {
        assert(value <= INT32_MAX && "Size too large for immediate");}
        
    // 整数立即数构造函数
    Immediate(int32_t value)
        : Operand(OpType::IntImm), 
          immType(ImmType::Int),
          intValue(value) {}

    // 浮点立即数构造函数
    Immediate(float value)
        : Operand(OpType::FloatImm),
          immType(ImmType::Float),
          floatValue(value) {}

    // 标签偏移构造函数
    Immediate(const std::string& label, int32_t offset = 0)
        : Operand(OpType::Label),
          immType(ImmType::LabelOffset),
          labelValue(label),
          offsetValue(offset) {}

    // 高低位分裂构造函数
    Immediate(std::shared_ptr<Immediate> origin, SplitPart part)
        : Operand(part == SplitPart::HI20 ? OpType::IntImm : OpType::IntImm),
          immType(ImmType::HiLoSplit),
          originImm(std::move(origin)),
          splitPart(part) {
        assert(originImm && "Origin immediate cannot be null");
    }

    // 获取立即数类型
    ImmType getImmType() const { return immType; }

    // 获取整数值（仅对Int类型有效）
    int32_t getIntValue() const {
        assert(immType == ImmType::Int && "Not an integer immediate");
        return intValue;
    }

    // 获取浮点值（仅对Float类型有效）
    float getFloatValue() const {
        assert(immType == ImmType::Float && "Not a float immediate");
        return floatValue;
    }

    // 获取标签值（仅对LabelOffset类型有效）
    const std::string& getLabelValue() const {
        assert(immType == ImmType::LabelOffset && "Not a label offset");
        return labelValue;
    }

    // 获取偏移值（仅对LabelOffset类型有效）
    int32_t getOffsetValue() const {
        assert(immType == ImmType::LabelOffset && "Not a label offset");
        return offsetValue;
    }

    // 获取原始立即数（仅对HiLoSplit类型有效）
    std::shared_ptr<Immediate> getOriginImm() const {
        assert(immType == ImmType::HiLoSplit && "Not a split immediate");
        return originImm;
    }

    // 获取分裂部分（仅对HiLoSplit类型有效）
    SplitPart getSplitPart() const {
        assert(immType == ImmType::HiLoSplit && "Not a split immediate");
        return splitPart;
    }

    // 计算实际值（用于代码生成）
    int32_t getEncodedValue() const {
        switch (immType) {
            case ImmType::Int:
                return intValue;
            case ImmType::Float:
                return *reinterpret_cast<const int32_t*>(&floatValue);
            case ImmType::LabelOffset:
                return offsetValue;  // 实际处理需要重定位
            case ImmType::HiLoSplit:
                return originImm->getEncodedValue();  // 由具体指令处理高低位
            default:
                assert(false && "Unknown immediate type");
                return 0;
        }
    }

    std::string toString() const override {
        switch (immType) {
            case ImmType::Int:
                return std::to_string(intValue);
            case ImmType::Float:
                return std::to_string(floatValue);
            case ImmType::LabelOffset:
                return labelValue + (offsetValue != 0 ? "+" + std::to_string(offsetValue) : "");
            case ImmType::HiLoSplit:
                return (splitPart == SplitPart::HI20 ? "%hi(" : "%lo(") + 
                       originImm->toString() + ")";
            default:
                return "unknown_immediate";
        }
    }

private:
    ImmType immType;
    union {
        int32_t intValue;
        float floatValue;
    };
    
    // 用于LabelOffset类型
    std::string labelValue;
    int32_t offsetValue;
    
    // 用于HiLoSplit类型
    std::shared_ptr<Immediate> originImm;
    SplitPart splitPart;
};

// 工厂函数
inline std::shared_ptr<Immediate> createIntImmediate(int32_t value) {
    return std::make_shared<Immediate>(value);
}

inline std::shared_ptr<Immediate> createFloatImmediate(float value) {
    return std::make_shared<Immediate>(value);
}

inline std::shared_ptr<Immediate> createLabelOffset(
    const std::string& label, int32_t offset = 0) {
    return std::make_shared<Immediate>(label, offset);
}

inline std::shared_ptr<Immediate> createHi20Split(
    std::shared_ptr<Immediate> origin) {
    return std::make_shared<Immediate>(std::move(origin), Immediate::SplitPart::HI20);
}

inline std::shared_ptr<Immediate> createLo12Split(
    std::shared_ptr<Immediate> origin) {
    return std::make_shared<Immediate>(std::move(origin), Immediate::SplitPart::LO12);
}

} // namespace backend