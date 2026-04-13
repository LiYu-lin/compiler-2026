#include "ir/IRBuilder.h"
#include "ir/value.h"
#define HANDLE_BINARY_CREATE(OPT, TYPE)                                              \
    Value *IRBuilder::Create##OPT(Value *lhs, Value *rhs, const char *c)             \
    {                                                                                \
        assert(lhs->getType()->is##TYPE##Ty() && rhs->getType()->is##TYPE##Ty());            \
        Value *value;                                                                \
        if ((value = folder->Fold##OPT##TYPE(lhs, rhs)) != nullptr)                  \
            return value;                                                            \
        return InsertBack(BinaryInstruction::create##OPT(lhs->getType(), lhs, rhs)); \
    }
#define HANDLE_UNARY_CREATE(OPT, TYPE)                                         \
    Value *IRBuilder::Create##OPT(Value *val, const char *c)                   \
    {                                                                          \
        assert(val->getType()->is##TYPE##Ty());                                    \
        Value *value;                                                          \
        if ((value = folder->Fold##OPT##TYPE(val)) != nullptr)                 \
            return value;                                                      \
        return InsertBack(UnaryInstruction::create##OPT(val->getType(), val)); \
    }

#define HANDLE_UNARY_POS(OPT, TYPE)                  \
    Value *IRBuilder::OPT(Value *val, const char *c) \
    {                                                \
        assert(val->getType()->is##TYPE##Ty());          \
        return val;                                  \
    }

#define HANDLE_CMP_CREATE(OPT, TYPE, CMPTYPE)                                    \
    Value *IRBuilder::Create##OPT(Value *lhs, Value *rhs, const char *c)         \
    {                                                                            \
        assert(lhs->getType()->is##TYPE##Ty() && rhs->getType()->is##TYPE##Ty());        \
        Value *value;                                                            \
        if ((value = folder->Fold##OPT##TYPE(lhs, rhs)) != nullptr)              \
            return value;                                                        \
        return InsertBack(CMPTYPE##Instruction::create##CMPTYPE##OPT(lhs, rhs)); \
    }

#define HANDLE_CAST_CREATE(OPT, TYPE, TOTYPE)                                                 \
    Value *IRBuilder::Create##OPT(Value *val, const char *c)                                  \
    {                                                                                          \
        assert(val->getType()->is##TYPE##Ty());                                               \
        Value *value;                                                                           \
        if ((value = folder->Fold##OPT(val)) != nullptr)                                      \
            return value;                                                                     \
        return InsertBack(CastInstruction::create##OPT(Type::get##TOTYPE##Type(), val)); \
    }

namespace IR
{
    AllocaInstruction *IRBuilder::CreateAlloca(pType type, std::string name, bool is_const, const char *c)
    {
        AllocaInstruction *temp = AllocaInstruction::create(type, name, is_const);
        InsertBack(temp);
        return temp;
    }

    AllocaInstruction *IRBuilder::CreateAlloca(pType type, std::string name, Constant *init, bool is_const, const char *c)
    {
        AllocaInstruction *temp = AllocaInstruction::Create(type, init, is_const, name);
        InsertBack(temp);
        return temp;
    }

    StoreInstruction *IRBuilder::CreateStore(Value *lval, Value *dest, const char *c)
    {
        StoreInstruction *temp = StoreInstruction::create(lval, dest);
        InsertBack(temp);
        return temp;
    }

    Value *IRBuilder::CreateLoad(pType ty, Value *op, const char *c)
    {
        assert(op->getType()->isPointerTy());
        Value *value;
        if ((value = folder->FoldLoad(ty, op)) != nullptr)
        {
            return value;
        }
        return InsertBack(LoadInstruction::create(ty, op));
    }

    Value *IRBuilder::CreateGEP(Value *base, std::vector<Value *> indices, const char *c)
    {
        assert(base->getType()->isPointerTy());
        Value *value;
        if ((value = folder->FoldGEP(base, indices)) != nullptr)
        {
            return value;
        }
        return InsertBack(GetElementPtrInstruction::create(base, indices, c));
    }

    Value *IRBuilder::CreateGEP(pType type, Value *base, std::vector<Value *> indices, const char *c)
    {
        assert(base->getType()->isPointerTy());
        Value *value;
        if ((value = folder->FoldGEP(base, indices)) != nullptr)
        {
            return value;
        }
        return InsertBack(GetElementPtrInstruction::create(type, base, indices, c));
    }

    // 函数调用
    CallInstruction *IRBuilder::CreateCall(Function *callee, std::vector<Value *> args, const char *c)
    {
        CallInstruction *callInstr = CallInstruction::create(callee, args);
        InsertBack(callInstr);
        return callInstr;
    }

    // 二元运算符
    HANDLE_BINARY_CREATE(Add, Int32)
    HANDLE_BINARY_CREATE(Sub, Int32)
    HANDLE_BINARY_CREATE(Mul, Int32)
    HANDLE_BINARY_CREATE(Div, Int32)
    HANDLE_BINARY_CREATE(Rem, Int32)

    HANDLE_BINARY_CREATE(FAdd, Float)
    HANDLE_BINARY_CREATE(FSub, Float)
    HANDLE_BINARY_CREATE(FMul, Float)
    HANDLE_BINARY_CREATE(FDiv, Float)

    HANDLE_BINARY_CREATE(And, Int32)
    HANDLE_BINARY_CREATE(Or, Int32)
    HANDLE_BINARY_CREATE(Xor, Int32)

    // 一元运算符

    HANDLE_UNARY_CREATE(Neg, Int32)
    HANDLE_UNARY_CREATE(FNeg, Float)
    HANDLE_UNARY_CREATE(Not, Int32)
    HANDLE_UNARY_POS(CreatePos, Int32)
    HANDLE_UNARY_POS(CreateFPos, Float)

    // 逻辑运算
    HANDLE_CMP_CREATE(Eq, Int32, ICmp)
    HANDLE_CMP_CREATE(Ne, Int32, ICmp)
    HANDLE_CMP_CREATE(Lt, Int32, ICmp)
    HANDLE_CMP_CREATE(Le, Int32, ICmp)
Value* IRBuilder::CreateGt(Value* lhs, Value* rhs, const char* c) {
    assert(lhs->getType()->isInt32Ty() && rhs->getType()->isInt32Ty());
    Value* value;
    if ((value = folder->FoldGtInt32(lhs, rhs)) != nullptr)
        return value;
    return InsertBack(ICmpInstruction::createICmpGt(lhs, rhs));
}
    HANDLE_CMP_CREATE(Ge, Int32, ICmp)

    HANDLE_CMP_CREATE(FEq, Float, FCmp)
    HANDLE_CMP_CREATE(FNe, Float, FCmp)
    HANDLE_CMP_CREATE(FLt, Float, FCmp)
    HANDLE_CMP_CREATE(FLe, Float, FCmp)
    HANDLE_CMP_CREATE(FGt, Float, FCmp)
    HANDLE_CMP_CREATE(FGe, Float, FCmp)


    Value *IRBuilder::CreateFPtoSI(Value *val, const char *c)
    {
        assert(val->getType()->isFloatTy());
        Value *value;
        if ((value = folder->FoldFPtoSI(val)) != nullptr)
            return value;
        return InsertBack(CastInstruction::createFPtoSI(Type::getI32Type(), val));
    }

    Value *IRBuilder::CreateSItoFP(Value *val, const char *c)
    {
        assert(val->getType()->isInt32Ty());
        Value *value;
        if ((value = folder->FoldSItoFP(val)) != nullptr)
            return value;
        return InsertBack(CastInstruction::createSItoFP(Type::getFloatType(), val));
    }
    // 控制流指令
    BranchInstruction *IRBuilder::CreateCondBr(Value *cond, BasicBlock *then_block, BasicBlock *else_block, const char *c)
    {
        BranchInstruction *temp = BranchInstruction::createCondBr(cond, then_block, else_block);
        InsertBack(temp);
        return temp;
    }

    BranchInstruction *IRBuilder::CreateBr(BasicBlock *dest, const char *c)
    {
        BranchInstruction *temp = BranchInstruction::createBr(dest);
        InsertBack(temp);
        return temp;
    }

    ReturnInstruction *IRBuilder::CreateRet(Value *val, Function *func, const char *c)
    {
        ReturnInstruction *temp = ReturnInstruction::create(val, func);
        InsertBack(temp);
        return temp;
    }

    ReturnInstruction *IRBuilder::CreateRetVoid(Function *func, const char *c)
    {
        ReturnInstruction *temp = ReturnInstruction::createVoid(func);
        InsertBack(temp);
        return temp;
    }

    // 插入指令至插入点
    Value *IRBuilder::InsertBack(Value *I)
    {
        assert(BB != nullptr);
        // if (!I->isInstruction())
        //     Error::Error("IRBuilder::InsertBack", "Value is not an instruction");
        BB->InsertInstructionBack(static_cast<Instruction *>(I));
        return I;
    }

    Value *IRBuilder::InsertFront(Value *I)
    {
        assert(BB != nullptr);
        // if (!I->isInstruction())
        //     Error::Error("IRBuilder::InsertFront", "Value is not an instruction");
        BB->InsertInstructionFront(static_cast<Instruction *>(I));
        return I;
    }

    Value *IRBuilder::Insert(Value *I)
    {
        // Error::Error("IRBuilder::Insert", "not implemented");
        //     if (IP == std::vector<)
        //     {
        //         Error::Error("IRBuilder::Insert", "Insert point is nullptr");
        //     }
        //     IP = BB->InsertInstruction(I, IP);
        return I;
    }
    
    void IRBuilder::SetInsertPoint(BasicBlock *bb) {
        BB = bb;
    }
    BasicBlock *IRBuilder::GetInsertBlock() {
        return BB;
    }
}


#undef HANDLE_BINARY_CREATE
#undef HANDLE_UNARY_CREATE
#undef HANDLE_UNARY_POS
#undef HANDLE_CMP_CREATE
#undef HANDLE_CAST_CREATE