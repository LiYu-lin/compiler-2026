#pragma once

#include <string>
#include <memory>
#include "RiscVOperand/OpLabel.h"

namespace backend {

inline unsigned convertFloatToU32(float val) {
    return *reinterpret_cast<unsigned*>(&val);
}

inline float convertU32ToFloat(unsigned uval) {
    return *reinterpret_cast<float*>(&uval);
}

class GlobalConstant {
public:
    enum StorageType {
        WORD_32BIT,
        DOUBLE_WORD_64BIT,
        ZERO_FILL
    };

protected:
    StorageType storageKind;
    int64_t Value;
    int fillLength;

public:
    GlobalConstant(StorageType kind, int64_t val, int len) 
        : storageKind(kind), Value(val), fillLength(len) {}
    
    explicit GlobalConstant(int val) 
        : storageKind(WORD_32BIT), Value(val), fillLength(0) {}
    
    explicit GlobalConstant(int64_t val) 
        : storageKind(DOUBLE_WORD_64BIT), Value(val), fillLength(0) {}
    
    explicit GlobalConstant(float val) 
        : storageKind(WORD_32BIT), Value(convertFloatToU32(val)), fillLength(0) {}

    StorageType getStorageType() const { return storageKind; }

    std::string output() const {
        switch (storageKind) {
        case WORD_32BIT:
            return "\t.word " + std::to_string(Value) + "\n";
        case DOUBLE_WORD_64BIT:
            return "\t.dword " + std::to_string(Value) + "\n";
        case ZERO_FILL:
            return "\t.zero " + std::to_string(fillLength) + "\n";
        default:
            return "";
        }
    }

    static GlobalConstant createZeroFill(int len) { 
        return GlobalConstant(ZERO_FILL, 0, len); 
    }
};

class FloatConstant {
    std::string constLabel;
    GlobalConstant constData;

public:
    explicit FloatConstant(float val) 
        : constLabel("float_const_" + std::to_string(convertFloatToU32(val))), 
          constData(val) {}

    std::string output() const {
        return "\t.align 2\n." + constLabel + ":\n" + constData.output();
    }

    std::shared_ptr<Label> createLabel() { 
        return std::make_shared<Label>(constLabel, false); 
    }
};

class LongIntConstant {
    std::string constLabel;
    GlobalConstant constData;

public:
    explicit LongIntConstant(int64_t val) 
        : constLabel("long_const_" + std::to_string(static_cast<uint64_t>(val))), 
          constData(val) {}

    std::string output() const {
        return "\t.align 4\n." + constLabel + ":\n" + constData.output();
    }

    std::shared_ptr<Label> createLabel() { 
        return std::make_shared<Label>(constLabel, false); 
    }
};

} // namespace backend