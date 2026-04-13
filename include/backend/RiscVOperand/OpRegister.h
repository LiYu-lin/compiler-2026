#pragma once
#include "BasicOperand.h"
#include "ir/Value.h"  
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>

namespace backend {
    class Instruction;

    class VirtualRegister : public Operand {
    private:
        IR::Value* irValue;  // 关联的IR值
        bool isTemp;         // 是否是临时寄存器
        std::string hint;    // 寄存器分配的提示
        bool isFloat;        // 是否是浮点寄存器  
        bool isConst = false;
    public:
        VirtualRegister(IR::Value* value, bool isFloat, bool temp = false, const std::string& h = "")
            : Operand(isFloat ? OpType::FloatReg : OpType::IntReg),
              irValue(value), isTemp(temp), hint(h), isFloat(isFloat) {}


        VirtualRegister(const VirtualRegister&) = delete;
        VirtualRegister& operator=(const VirtualRegister&) = delete;

        static std::shared_ptr<VirtualRegister> create(IR::Value* value, bool isFloat = false) {
            if (!value) {
                throw std::runtime_error("Cannot create virtual register for null IR value");
            }

            auto &cache = isFloat ? floatValueCache() : intValueCache();
            auto it = cache.find(value);
            if (it != cache.end()) {
                return it->second;
            }

            auto reg = std::make_shared<VirtualRegister>(value, isFloat);
            cache.emplace(value, reg);
            return reg;
        }
        
        static std::shared_ptr<VirtualRegister> createTemp(bool isFloat = false) {
            return std::make_shared<VirtualRegister>(nullptr, isFloat, true);
        }
        
        // 创建零寄存器(x0/f0)
        static std::shared_ptr<VirtualRegister> createZero(bool isFloat = false) {
            auto reg = std::make_shared<VirtualRegister>(nullptr, isFloat);
            reg->hint = isFloat ? "f0" : "x0";
            return reg;
        }
        
        // 创建返回值寄存器(a0/fa0)
        static std::shared_ptr<VirtualRegister> createReturnValue(bool isFloat = false) {
            auto reg = std::make_shared<VirtualRegister>(nullptr, isFloat);
            reg->hint = isFloat ? "fa0" : "a0";
            return reg;
        }
        static std::shared_ptr<VirtualRegister> createTemporary(bool isFloat = false) {
            return std::make_shared<VirtualRegister>(nullptr, isFloat, true);
        }
        // 获取关联的IR值
        IR::Value* getIRValue() const { return irValue; }
        
        // 是否是临时寄存器
        bool isTemporary() const { return isTemp; }
        
        const std::string& getHint() const { return hint; }
        
        bool isFloatReg() const { return isFloat; }
        bool isConstReg() const { return isConst; }
        void setConst(bool value) { isConst = value; }


        std::string toString() const override{
            if (!hint.empty()) {
                return hint;
            }
            if (!irValue) {
                return std::string(isFloat ? "vf_tmp" : "v_tmp");
            }
            return std::string(isFloat ? "vf" : "v") + irValue->getIRName();
        }
        
        static std::shared_ptr<VirtualRegister> createStackPointer() {
            auto reg = std::make_shared<VirtualRegister>(nullptr, false);
            reg->hint = "sp";
            return reg;
        }

        void setStackPointerRef(bool isRef) {
            if (isRef) {
                hint = "sp_ref";
            } else {
                hint = "sp";
            }
        }
        
        void setHint(const std::string& h) {
            hint = h;
        }

        static std::shared_ptr<VirtualRegister> createStackPointerRef() {
            static auto spRef = std::make_shared<VirtualRegister>(nullptr, false);
            spRef->setHint("sp");
            return spRef;
        }

        std::unordered_set<Instruction*> defUsers;  
        std::unordered_set<Instruction*> useUsers;

    private:
        static std::unordered_map<IR::Value*, std::shared_ptr<VirtualRegister>>& intValueCache() {
            static std::unordered_map<IR::Value*, std::shared_ptr<VirtualRegister>> cache;
            return cache;
        }

        static std::unordered_map<IR::Value*, std::shared_ptr<VirtualRegister>>& floatValueCache() {
            static std::unordered_map<IR::Value*, std::shared_ptr<VirtualRegister>> cache;
            return cache;
        }
    };

    // 物理寄存器实现（保留原有设计，但适配新接口）
    class PhysicalRegister : public Operand {
    private:
        int regIndex;
        bool isFloat;
        std::string name;

        inline static std::unordered_map<int, std::shared_ptr<PhysicalRegister>> intCache;
        inline static std::unordered_map<int, std::shared_ptr<PhysicalRegister>> floatCache;

    public:
        PhysicalRegister(int regIndex, bool isFloat)
            : Operand(isFloat ? OpType::FloatReg : OpType::IntReg),
              regIndex(regIndex), isFloat(isFloat) {
            if (isFloat) {
                static const char* floatNames[] = {
                    "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",
                    "fs0", "fs1", "fa0", "fa1", "fa2", "fa3", "fa4", "fa5",
                    "fa6", "fa7", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7",
                    "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"
                };
                name = (regIndex >= 0 && regIndex < 32) ? floatNames[regIndex]
                                                        : "f" + std::to_string(regIndex);
            } else {
                static const char* intNames[] = {
                    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
                };
                name = (regIndex >= 0 && regIndex < 32) ? intNames[regIndex]
                                                        : "x" + std::to_string(regIndex);
            }
        }

        int getId() const { return regIndex; }
        bool isFloatReg() const { return isFloat; }
        std::string toString() const override{ return name; }

        static std::shared_ptr<PhysicalRegister> get(int regIndex, bool isFloat = false) {
            auto& cache = isFloat ? floatCache : intCache;
            if (cache.find(regIndex) == cache.end()) {
                cache[regIndex] = std::make_shared<PhysicalRegister>(regIndex, isFloat);
            }
            return cache[regIndex];
        }

        static std::shared_ptr<PhysicalRegister> getParamReg(int paramIndex, bool isFloat = false) {
            if (paramIndex < 0 || paramIndex >= 8) {
                throw std::runtime_error("Invalid parameter index");
            }
            return get(10 + paramIndex, isFloat); // a0-a7对应x10-x17, fa0-fa7对应f10-f17
        }
    };


    using rRegister = std::shared_ptr<VirtualRegister>;
    using pRegister = std::shared_ptr<PhysicalRegister>;
} // namespace backend
