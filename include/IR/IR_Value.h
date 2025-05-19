/**
 * @file IR_Value.h
 * @author CoffeeRain (CoffeeRain123@gmail.com)
 * @brief A file containing the definition of the IR values.
 * @version 0.1
 * @date 2025-05-19
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef IR_Value_H
#define IR_Value_H
#include "Parse/AST.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>


  namespace IR {
    // IR的统一接口
    struct IRValue {
        enum class ValueType {
            //Constant
            FUNC_VALUE,
            TEMP_VALUE,
            GLOBALVAR_VALUE,
            CONSTANTINT_VALUE,
            CONSTANTFLOAT_VALUE,
            CONSTANTARRAY_VALUE,
            CONSTANTPOINTER_VALUE,
            // Value
            ARGUMENT_VALUE,
            BASICBLOCK_VALUE,
            //Instructions
            INSTRUCTION_VALUE,
        };
        
        ValueType type;
        std::string name;  // 可用于调试或输出
        int constant = 0;  

        IRValue(ValueType t, const std::string& n, int c = 0)
            : type(t), name(n), constant(c) {}
        //构造省略一下
    };

    //IR指令基类
    struct IRInst {
        enum class InstType {
            BinaryOp, Assign, Load, Store, Br, CondBr, Call, Ret
        };
        
        InstType type;

        IRInst(InstType t) : type(t) {}
        virtual ~IRInst() = default;
    };

    // 二元操作指令
    struct IRBinaryOp : IRInst {
        enum class OpType { Add, Sub, Mul, Div, Mod, Lt, Gt, Eq, Ne, And, Or };

        OpType op;
        IRValue dst, lhs, rhs;

        IRBinaryOp(OpType o, const IRValue& d, const IRValue& l, const IRValue& r)
            : IRInst(InstType::BinaryOp), op(o), dst(d), lhs(l), rhs(r) {}
    };

    // 单值赋值指令
    struct IRAssign : IRInst {
        IRValue dst, src;

        IRAssign(const IRValue& d, const IRValue& s)
            : IRInst(InstType::Assign), dst(d), src(s) {}
    };

    // 条件跳转
    struct IRCondBr : IRInst {
        IRValue cond;
        std::string trueLabel;
        std::string falseLabel;

        IRCondBr(const IRValue& c, const std::string& t, const std::string& f)
            : IRInst(InstType::CondBr), cond(c), trueLabel(t), falseLabel(f) {}
    };

    // 无条件跳转
    struct IRBr : IRInst {
        std::string targetLabel;

        IRBr(const std::string& tgt)
            : IRInst(InstType::Br), targetLabel(tgt) {}
    };

    // 返回指令
    struct IRRet : IRInst {
        //使用optional来表示可能没有返回值
        std::optional<IRValue> retVal;

        IRRet() : IRInst(InstType::Ret), retVal(std::nullopt) {}
        IRRet(const IRValue& val) : IRInst(InstType::Ret), retVal(val) {}
    };

    // 基本块，包含一个指令集
    struct IRBlock {
        std::string label;
        //使用智能指针
        std::vector<std::unique_ptr<IRInst>> instructions;

        IRBlock(const std::string& lbl) : label(lbl) {}

        void addInst(std::unique_ptr<IRInst> inst) {
            instructions.emplace_back(std::move(inst));
        }
    };

    // 函数
    struct IRFunction {
        std::string name;
        std::vector<IRValue> args;
        std::vector<std::unique_ptr<IRBlock>> blocks;
        //禁止隐式转换
        explicit IRFunction(const std::string& n) : name(n) {}

        IRBlock* addBlock(const std::string& label) {
            blocks.emplace_back(std::make_unique<IRBlock>(label));
            return blocks.back().get();
        }
    };

    // 一个编译模块
    struct IRModule {
        std::vector<std::unique_ptr<IRFunction>> functions;

        IRFunction* addFunction(const std::string& name) {
            functions.emplace_back(std::make_unique<IRFunction>(name));
            return functions.back().get();
        }
    };

  }


#endif 

