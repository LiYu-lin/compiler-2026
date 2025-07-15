#include "ir/type.h"
#include <cassert>
// Type
namespace IR {
pType Type::getVoidType() {
    static Type type(VOID);
    return &type;
}

pType Type::getLabelType() {
    static Type type(LABEL);
    return &type;
}

pIntegerType Type::getI32Type() { return IntegerType::getIntegerType(32); }


pType Type::getFloatType() {
    static Type type(FLOAT);
    return &type;
}

pPointerType Type::getPointerType(pType base) { return PointerType::getPointerType(base); }


int Type::getIntegerBits() const {
    assert(isInt32Ty());
    return static_cast<pIntegerType>(this)->bits; 
}

pType Type::getPointerBase() const {
    if (!isPointerTy()) {
        assert(false && "Error: Expected pointer type but got ");
        return nullptr;
    }
    return static_cast<pPointerType>(this)->base;
}

int Type::getArraySize() const {
    assert(isArrayTy());
    return static_cast<pArrayType>(this)->size; 
}

pType Type::getArrayBase() const {
    assert(isArrayTy());
    return static_cast<pArrayType>(this)->base; 
}

pType Type::getBase() const {
    if (isPointerTy()) return getPointerBase();
    if (isArrayTy()) return getArrayBase();
    return nullptr;
}

pType Type::getBaseRecursive() const {
    if (auto ty = getBase()) return ty->getBaseRecursive();
    return this;
}

pType Type::getFunctionRet() const {
    assert(isFunctionTy());
    return static_cast<pFunctionType>(this)->ret; 
}

const std::vector<pType> &Type::getFunctionParams() const {
    assert(isFunctionTy());
    return static_cast<pFunctionType>(this)->params; 
}

pType Type::getFunctionParam(int i) const {
    assert(isFunctionTy());
    return static_cast<pFunctionType>(this)->params[i];  
}

int Type::getFunctionParamCount() const {
    assert(isFunctionTy());
    return static_cast<pFunctionType>(this)->params.size(); 
}

bool Type::convertableTo(pType other) const {
    if (this == other) return true;
    if (isInt32Ty() || isFloatTy()) return other->isInt32Ty() || other->isFloatTy();
    if ((isArrayTy() || isPointerTy()) && other->isPointerTy()) return true;
    return false;
}


std::string Type::to_string() const {
    using namespace std::string_literals;
    if (isLabelTy()) return "<label>"s;
    if (isVoidTy()) return "void"s;
    if (isFloatTy()) return "float"s;
    if (isInt32Ty()) return "i" + std::to_string(getIntegerBits());
    if (isPointerTy()) return "ptr"s;
    if (isArrayTy())
        return "[" + std::to_string(getArraySize()) + " x " + getArrayBase()->to_string() + "]";
    if (isFunctionTy()) {
        std::string ret = getFunctionRet()->to_string();
        std::string params;
        for (auto param : getFunctionParams()) {
            params += param->to_string() + ", ";
        }
        if (!params.empty()) params.pop_back(), params.pop_back();
        return ret + " (" + params + ")";
    }
    return "<unknown>"s;
}
}  // namespace IR

#include <unordered_map>

template <typename First>
struct [[maybe_unused]] std::hash<std::pair<First, IR::pType>> {
    size_t operator()(const std::pair<First, IR::pType> &p) const {
        return std::hash<First>()(p.first) * 10007 + std::hash<IR::pType>()(p.second);
    }
};

template <typename Second>
struct [[maybe_unused]] std::hash<std::pair<IR::pType, Second>> {
    size_t operator()(const std::pair<IR::pType, Second> &p) const {
        return std::hash<IR::pType>()(p.first) * 10007 + std::hash<Second>()(p.second);
    }
};

template <>
struct [[maybe_unused]] std::hash<std::vector<IR::pType>> {
    size_t operator()(const std::vector<IR::pType> &v) const noexcept {
        size_t ret = 0;
        for (const auto &e : v) {
            ret = ret * 10007 + std::hash<IR::pType>()(e);
        }
        return ret;
    }
};

// Derived types
namespace IR {
template <typename K, typename V>
using map = std::unordered_map<K, V>;

pIntegerType IntegerType::getIntegerType(int bits) {
    static map<int, pIntegerType> cache;
    if (cache.find(bits) == cache.end()) {
        cache[bits] = new IntegerType(bits);
    }
    return cache[bits];
}

pPointerType PointerType::getPointerType(pType base) {
    static map<pType, pPointerType> cache;
    if (cache.find(base) == cache.end()) {
        cache[base] = new PointerType(base);
    }
    return cache[base];
}

pArrayType ArrayType::getArrayType(int size, pType base) {
    static map<std::pair<int, pType>, pArrayType> cache;
    std::pair key{size, base};
    if (cache.find(key) == cache.end()) {
        cache[key] = new ArrayType(size, base);
    }
    return cache[key];
}

pFunctionType FunctionType::getFunctionType(pType ret, std::vector<pType> &&params) {
    static map<std::pair<pType, std::vector<pType>>, pFunctionType> cache;
    std::pair key{ret, params};
    if (cache.find(key) == cache.end()) {
        cache[key] = new FunctionType(ret, std::move(params));
    }
    return cache[key];
}

}  // namespace IR