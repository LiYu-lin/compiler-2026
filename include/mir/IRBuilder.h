#pragma once
#include "value.h"
#include "type.h"
#include "mir/Value/instruction.h"
#include "basicblock.h"

namespace IR
{
    struct IRBuilder
    {
        BasicBlock *BB;

        IRBuilder() : BB(nullptr) {}

        // 设置插入点
        void SetInsertPoint(BasicBlock *I)
        {
            BB = I;
        }

        BasicBlock *GetInsertBlock() { return BB; }

        // 插入指令到插入点
        Value *Insert(Value *I)
        {
            assert(BB && "Insert point not set!");
            auto *inst = dynamic_cast<Instruction *>(I);
            assert(inst && "Insert value must be an Instruction!");
            BB->InsertInstructionBack(inst);
            return I;
        }

        // 内存管理指令
        AllocaInstruction *CreateAlloca(Type *type, std::string name, bool is_const = false, const char *c = "")
        {
            auto *inst = AllocaInstruction::create(type, name, is_const);
            Insert(inst);
            return inst;
        }

        AllocaInstruction *CreateAlloca(Type *type, std::string name, Constant *init, bool is_const, const char *c = "")
        {
            auto *inst = AllocaInstruction::Create(type, init, is_const, name);
            Insert(inst);
            return inst;
        }

        Value *CreateGEP(Value *base, std::vector<Value *> indices, const char *c = "")
        {
            auto *inst = GetElementPtrInstruction::create(base, indices);
            Insert(inst);
            return inst;
        }

        Value *CreateGEP(pType type, Value *base, std::vector<Value *> indices, const char *c = "")
        {
            auto *inst = GetElementPtrInstruction::create(type, base, indices);
            Insert(inst);
            return inst;
        }

        StoreInstruction *CreateStore(Value *lval, Value *rval, const char *c = "")
        {
            auto *inst = StoreInstruction::create(rval, lval);
            Insert(inst);
            return inst;
        }

        Value *CreateLoad(Type *ty, Value *op, const char *c = "")
        {
            auto *inst = new LoadInstruction(ty, op, c ? c : "");
            Insert(inst);
            return inst;
        }

        // 二元运算符
        Value *CreateAdd(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createAdd(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateSub(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createSub(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateMul(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createMul(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateDiv(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createDiv(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateRem(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createRem(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }

        Value *CreateFAdd(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createFAdd(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateFSub(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createFSub(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateFMul(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createFMul(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateFDiv(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createFDiv(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }

        // 一元运算符
        Value *CreateNeg(Value *val, const char *c = "")
        {
            auto *inst = UnaryInstruction::createNeg(val->type, val);
            Insert(inst);
            return inst;
        }
        Value *CreateFNeg(Value *val, const char *c = "")
        {
            auto *inst = UnaryInstruction::createFNeg(val->type, val);
            Insert(inst);
            return inst;
        }
        Value *CreateNot(Value *val, const char *c = "")
        {
            auto *inst = UnaryInstruction::createNot(Type::getI32Type(), val);
            Insert(inst);
            return inst;
        }
        Value *CreatePos(Value *val, const char *c = "")
        {
            return val;
        }
        Value *CreateFPos(Value *val, const char *c = "")
        {
            return val;
        }

        // 位运算符
        Value *CreateAnd(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createAnd(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateOr(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createOr(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }
        Value *CreateXor(Value *lhs, Value *rhs, const char *c = "")
        {
            auto *inst = BinaryInstruction::createXor(lhs->type, lhs, rhs);
            Insert(inst);
            return inst;
        }

        // 逻辑运算
        Value *CreateFEq(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateFNe(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateFLt(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateFLe(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateFGt(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateFGe(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateEq(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateNe(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateLt(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateLe(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateGt(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateGe(Value *lhs, Value *rhs, const char *c = "");

        // 函数调用
        CallInstruction *CreateCall(Function *callee, std::vector<Value *> args, const char *c = "");

        // 类型转
        Value *CreateFPtoSI(Value *val, const char *c = "")
        {
            auto *inst = CastInstruction::createFPtoSI(Type::getI32Type(), val);
            Insert(inst);
            return inst;
        }
        Value *CreateSItoFP(Value *val, const char *c = "")
        {
            auto *inst = CastInstruction::createSItoFP(Type::getFloatType(), val);
            Insert(inst);
            return inst;
        }

        // 控制流指令
        BranchInstruction *CreateCondBr(Value *cond, BasicBlock *then_block, BasicBlock *else_block, const char *c = "");
        BranchInstruction *CreateBr(BasicBlock *dest, const char *c = "");
        ReturnInstruction *CreateRet(Value *val, Function *func, const char *c = "");
        ReturnInstruction *CreateRetVoid(Function *func, const char *c = "");

        // 插入指令至插入点（前/后）
        Value *InsertBack(Value *I)
        {
            assert(BB && "Insert point not set!");
            auto *inst = dynamic_cast<Instruction *>(I);
            assert(inst && "Insert value must be an Instruction!");
            BB->InsertInstructionBack(inst);
            return I;
        }
        Value *InsertFront(Value *I)
        {
            assert(BB && "Insert point not set!");
            auto *inst = dynamic_cast<Instruction *>(I);
            assert(inst && "Insert value must be an Instruction!");
            BB->InsertInstructionFront(inst);
            return I;
        }
    };

}