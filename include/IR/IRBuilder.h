#pragma once
#include "value.h"
#include "type.h"
#include "ir/Value/instruction.h"
#include "basicblock.h"
#include "ir/opt/constantfolder.h"

namespace IR
{
    struct IRBuilder
    {
        BasicBlock *BB;
        ConstantFolder *folder;
        IRBuilder()
        {
            BB = nullptr;
            folder = ConstantFolder::get();
        }
        // 设置插入点
        void SetInsertPoint(BasicBlock *I);

        BasicBlock *GetInsertBlock();

        // 插入指令到插入点
        Value *Insert(Value *I);

        // 内存管理指令
        AllocaInstruction *CreateAlloca(pType type, std::string name, bool is_const = false, const char *c = "");
        AllocaInstruction *CreateAlloca(pType type, std::string name, Constant *init, bool is_const, const char *c = "");

        Value *CreateGEP(Value *base, std::vector<Value *> indices, const char *c = "");
        Value *CreateGEP(pType type, Value *base, std::vector<Value *> indices, const char *c = "");

        StoreInstruction *CreateStore(Value *lval, Value *rval, const char *c = "");
        Value *CreateLoad(Type *ty, Value *op, const char *c = "");

        // 二元运算符
        Value *CreateAdd(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateSub(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateMul(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateDiv(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateRem(Value *lhs, Value *rhs, const char *c = "");

        Value *CreateFAdd(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateFSub(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateFMul(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateFDiv(Value *lhs, Value *rhs, const char *c = "");

        // 一元运算符
        Value *CreateNeg(Value *val, const char *c = "");
        Value *CreateFNeg(Value *val, const char *c = "");
        Value *CreateNot(Value *val, const char *c = "");
        Value *CreatePos(Value *val, const char *c = "");
        Value *CreateFPos(Value *val, const char *c = "");

        // 位运算符
        Value *CreateAnd(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateOr(Value *lhs, Value *rhs, const char *c = "");
        Value *CreateXor(Value *lhs, Value *rhs, const char *c = "");

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
        Value *CreateFPtoSI(Value *val, const char *c = "");
        Value *CreateSItoFP(Value *val, const char *c = "");

        // 控制流指令
        BranchInstruction *CreateCondBr(Value *cond, BasicBlock *then_block, BasicBlock *else_block, const char *c = "");
        BranchInstruction *CreateBr(BasicBlock *dest, const char *c = "");
        ReturnInstruction *CreateRet(Value *val, Function *func, const char *c = "");
        ReturnInstruction *CreateRetVoid(Function *func, const char *c = "");

        // 插入指令至插入点（前/后）
        Value *InsertBack(Value *I);
        Value *InsertFront(Value *I);
    };
}