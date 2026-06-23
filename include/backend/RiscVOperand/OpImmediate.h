#pragma once
#include "BasicOperand.h"  
#include <memory>
#include <utility>
#include <cassert>
#include <cstdint>
#include <cstring>
namespace backend {

class Immediate : public Operand {
public:
    enum class ImmType {
        Int,
        Float,
        LabelOffset,  
        HiLoSplit     
    };

    enum class SplitPart { HI20, LO12 };

    // 适配 RV64：全量升级为 int64_t 存储，杜绝大指针截断
    Immediate(size_t value)
        : Operand(OpType::IntImm),
          immType(ImmType::Int),
          int64Value(static_cast<int64_t>(value)) {}
         
    Immediate(int32_t value)
        : Operand(OpType::IntImm), 
          immType(ImmType::Int),
          int64Value(static_cast<int64_t>(value)) {}

    Immediate(int64_t value)
        : Operand(OpType::IntImm), 
          immType(ImmType::Int),
          int64Value(value) {}

    Immediate(float value)
        : Operand(OpType::FloatImm),
          immType(ImmType::Float),
          floatValue(value) {}

    Immediate(const std::string& label, int32_t offset = 0)
        : Operand(OpType::Label),
          immType(ImmType::LabelOffset),
          int64Value(0), // 强行对齐 Union 空间，防止脏数据
          labelValue(label),
          offsetValue(offset) {}

    Immediate(std::shared_ptr<Immediate> origin, SplitPart part)
        : Operand(OpType::IntImm),
          immType(ImmType::HiLoSplit),
          int64Value(0),
          originImm(std::move(origin)),
          splitPart(part) {
        assert(originImm && "Origin immediate cannot be null");
    }

    ImmType getImmType() const { return immType; }

    int64_t getIntValue() const {
        assert(immType == ImmType::Int && "Not an integer immediate");
        return int64Value;
    }

    float getFloatValue() const {
        assert(immType == ImmType::Float && "Not a float immediate");
        return floatValue;
    }

    const std::string& getLabelValue() const {
        assert(immType == ImmType::LabelOffset && "Not a label offset");
        return labelValue;
    }

    int32_t getOffsetValue() const {
        assert(immType == ImmType::LabelOffset && "Not a label offset");
        return offsetValue;
    }

    std::shared_ptr<Immediate> getOriginImm() const {
        assert(immType == ImmType::HiLoSplit && "Not a split immediate");
        return originImm;
    }

    SplitPart getSplitPart() const {
        assert(immType == ImmType::HiLoSplit && "Not a split immediate");
        return splitPart;
    }

    int64_t getEncodedValue() const {
        switch (immType) {
            case ImmType::Int:
                return int64Value;
            case ImmType::Float: {
                int32_t bits = 0;
                std::memcpy(&bits, &floatValue, sizeof(float));
                return static_cast<int64_t>(bits);
            }
            case ImmType::LabelOffset:
                return static_cast<int64_t>(offsetValue);  
            case ImmType::HiLoSplit:
                return originImm->getEncodedValue();  
            default:
                return 0;
        }
    }

    std::string toString() const override {
        switch (immType) {
            case ImmType::Int:
                return std::to_string(int64Value);
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
        int64_t int64Value;
        float floatValue;
    };
    
    std::string labelValue;
    int32_t offsetValue;
    
    std::shared_ptr<Immediate> originImm;
    SplitPart splitPart;
};

inline std::shared_ptr<Immediate> createIntImmediate(int64_t value) {
    return std::make_shared<Immediate>(value);
}
inline std::shared_ptr<Immediate> createFloatImmediate(float value) {
    return std::make_shared<Immediate>(value);
}
inline std::shared_ptr<Immediate> createLabelOffset(const std::string& label, int32_t offset = 0) {
    return std::make_shared<Immediate>(label, offset);
}
inline std::shared_ptr<Immediate> createHi20Split(std::shared_ptr<Immediate> origin) {
    return std::make_shared<Immediate>(std::move(origin), Immediate::SplitPart::HI20);
}
inline std::shared_ptr<Immediate> createLo12Split(std::shared_ptr<Immediate> origin) {
    return std::make_shared<Immediate>(std::move(origin), Immediate::SplitPart::LO12);
}

} // namespace backend