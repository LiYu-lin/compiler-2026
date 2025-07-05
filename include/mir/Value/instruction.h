#pragma once
#include "mir/type.h"
#include "mir/Value/Constant.h"
#include "user.h"
#include "mir/listnode.h"
#include "mir/list.h"
#include "globalvalue.h"
#include "mir/basicblock.h"

#define HANDLE_BINARY_CREATE(op)                                                \
    static BinaryInstruction *create##op(pType ty, Value *lhs, Value *rhs) \
    {                                                                           \
        return new BinaryInstruction(ty, BinaryOp::op, lhs, rhs);               \
    }
#define HANDLE_UNARY_CREATE(op)                                   \
    static UnaryInstruction *create##op(pType ty, Value *op) \
    {                                                             \
        return new UnaryInstruction(ty, UnaryOp::op, op);         \
    }
#define HANDLE_CAST_CREATE(op)                                   \
    static CastInstruction *create##op(pType ty, Value *op) \
    {                                                            \
        return new CastInstruction(ty, CastOp::op, op);          \
    }
#define HANDLE_CMP_INST(TYPE, OPCODE)                                      \
    static TYPE##Instruction *create##TYPE##OPCODE(Value *lhs, Value *rhs) \
    {                                                                      \
        return TYPE##Instruction::create(OPCODE, lhs, rhs);                \
    }
#define HANDLE_CMP_CREATE(op) \
    static BinaryInstruction *create##op(pType ty, Value *lhs, Value *rhs) \
    { \
        return new BinaryInstruction(ty, BinaryOp::op, lhs, rhs); \
    }

using PVB = std::pair<IR::Value *, IR::BasicBlock *>;

namespace IR
{
    struct Instruction: public User,public ListNode
    {
        virtual ~Instruction() = default;

        BasicBlock *parent = nullptr;

        Instruction(pType type) : User(type, Value::InstructionVal), ListNode(1) {}

        Instruction(pType type, const unsigned int opCode) : User(type, Value::InstructionVal + opCode), ListNode(1) {}

        BasicBlock *getParentBB() const { return parent; }

        bool isUseless();

        Value *getOperand(unsigned int i) const override { return operands[i]; }

        
        std::string getIRName() const override
        {
            return "@" + std::to_string(number);
        };

        enum BinaryOp
        {
            // 二元运算符
            BinaryBegin = 3,
            Add = 3,
            Sub = 4,
            Mul = 5,
            Div = 6,
            Rem = 7,
            FAdd = 8,
            FSub = 9,
            FMul = 10,
            FDiv = 11,
            FRem = 12,
            // 位运算符
            And = 13,
            Or = 14,
            Xor = 15,
            BinaryEnd = 16,
        };

        enum UnaryOp
        {
            // 一元运算符
            UnaryBegin = 16,
            Neg = 16,
            FNeg = 17,
            Not = 18,
            UnaryEnd = 19,
        };

        enum MemoryOp
        {
            // 内存操作
            MemoryBegin = 19,
            Alloca = 19,
            Load = 20,
            Store = 21,
            GEP = 22,
            MemoryEnd = 23,
        };

        enum CastOp
        {
            // 类型转换
            CastBegin = 23,
            FPtoSI = 23,
            SItoFP = 24,
            CastEnd = 25,
        };

        enum CmpOp
        {
            // 比较操作
            CmpBegin = 25,
            ICmp = 25,
            Ne = 26,
            Lt = 27,
            Le = 28,
            Gt = 29,
            Ge = 30,
            Eq = 31,
            FCmp = 32,
            FNe = 33,
            FLt = 34,
            FLe = 35,
            FGt = 36,
            FGe = 37,
            FEq = 38,
            CmpEnd = 39,
        };

        enum OtherOp
        {
            // 其他操作
            OtherOp = 39,
            Call = 40,
            Phi = 41,
            Select = 42,
            OtherEnd = 43,
        };

        enum TerminatorOp
        {
            TerminatorBegin = 43,
            Return = 43,
            BR = 44,
            InderectBr = 45,
            TerminatorEnd = 46
        };

        unsigned int getOpcode()
        {
            return subClassID - Value::InstructionVal;
        }

        std::string getOpStr();

        void waste() override;

        bool isTerminator()
        {
            return getOpcode() >= TerminatorBegin && getOpcode() <= TerminatorEnd;
        }

    private:
        friend struct List<Instruction *>;
        void removeNode() override;
    };

        struct BinaryInstruction : public Instruction
    {
        BinaryInstruction(pType ty, BinaryOp op, Value *lhser, Value *rhser);
        // x = op type op1 op2
        void emitIR(std::ostream &os)
        {
            os << getIRName() + " = " + getOpStr() + " " + type->getTypeName() + " " + getOperand(0)->getIRName() + " " + getOperand(1)->getIRName() + "\n";
        }

        static BinaryInstruction *create(pType ty, BinaryOp op, Value *lhs, Value *rhs)
        {
            return new BinaryInstruction(ty, op, lhs, rhs);
        }

        HANDLE_BINARY_CREATE(Add)
        HANDLE_BINARY_CREATE(Sub)
        HANDLE_BINARY_CREATE(Mul)
        HANDLE_BINARY_CREATE(Div)
        HANDLE_BINARY_CREATE(Rem)
        HANDLE_BINARY_CREATE(And)
        HANDLE_BINARY_CREATE(Or)
        HANDLE_BINARY_CREATE(Xor)
        HANDLE_BINARY_CREATE(FAdd)
        HANDLE_BINARY_CREATE(FSub)
        HANDLE_BINARY_CREATE(FMul)
        HANDLE_BINARY_CREATE(FDiv)
        HANDLE_BINARY_CREATE(FRem)
    };

        struct UnaryInstruction : public Instruction
    {
        UnaryInstruction(pType type, const unsigned int x, Value *op);
        UnaryInstruction(pType type, UnaryOp x, Value *op);

        void emitIR(std::ostream &os) override
        {
            os << getIRName() << " = " << getOpStr() << " " << getOperand(0)->getIRName() << std::endl;
        }

        static UnaryInstruction *create(pType type, UnaryOp x, Value *op)
        {
            return new UnaryInstruction(type, x, op);
        }

        HANDLE_UNARY_CREATE(Neg)
        HANDLE_UNARY_CREATE(FNeg)
        static UnaryInstruction *createNot(pType ty, Value *op)
        {
            return UnaryInstruction::create(Type::getI32Type(), UnaryOp::Not, op);
        }
    };

    struct CastInstruction : public UnaryInstruction
    {
        CastInstruction(pType type, CastOp x, Value *op);
        CastInstruction(pType type, const unsigned int x, Value *op);

        static CastInstruction *create(pType type, CastOp x, Value *op)
        {
            return new CastInstruction(type, x, op);
        }
        HANDLE_CAST_CREATE(FPtoSI)
        HANDLE_CAST_CREATE(SItoFP)
    };

    struct AllocaInstruction : public Instruction
    {
        bool isConst;
        AllocaInstruction(pType ty, std::string name, bool isConst = false);

        AllocaInstruction(pPointerType type, std::string name, Constant *init, bool isConst);

        static AllocaInstruction *create(pType type, std::string name = "", bool isConst = false)
        {
            pPointerType ptrType = PointerType::getPointerType(type);
            return new AllocaInstruction(ptrType, name, isConst);
        }

        static AllocaInstruction *Create(pType type, Constant *init, bool isConst, std::string name = "")
        {
            pPointerType ptrType = PointerType::getPointerType(type);
            return new AllocaInstruction(ptrType, name, init, isConst);
        }

        void setInitializer(Constant *init) override
        {
            assert(operands.size() > 0);
            operands[0] = static_cast<Value*>(init);
        }

        void emitIR(std::ostream &os) override
        {
            os << getIRName() << " = " << getOpStr() << " " << type->getPointerBase()->getTypeName() << std::endl;
        }

        pType getAllocaType() { return type->getPointerBase(); }
        bool isPointingConst() { return isConst; }
    };

    struct LoadInstruction : public UnaryInstruction
    {
        LoadInstruction(pType type, Value *op, std::string name)
            : UnaryInstruction(type, MemoryOp::Load, op)
        {
            this->name = name;
        }

        static LoadInstruction *create(pType type, Value *op, std::string name = "")
        {
            return new LoadInstruction(type, op, name);
        }

        void emitIR(std::ostream &os) override
        {
            std::string lhs = getIRName();
            std::string op = getOperand(0)->getTypeName() + " " + getOperand(0)->getIRName();
            os << lhs << " = " << getOpStr() + " " + getTypeName() << ", " << op << std::endl;
        }

        Value *getSrc() { return getOperand(0); }
    };


    struct StoreInstruction : public Instruction
    {
        StoreInstruction(Value *lhs, Value *dest);

        void emitIR(std::ostream &os) override
        {
            std::string lhs = getOperand(0)->getTypeName() + " " + getOperand(0)->getIRName();
            std::string dest = getOperand(1)->getTypeName() + " " + getOperand(1)->getIRName();
            os << getOpStr() << " " << lhs << ", " << dest << std::endl;
        }

        static StoreInstruction *create(Value *lhs, Value *dest)
        {
            return new StoreInstruction(lhs, dest);
        }

        Value *getSrc() { return getOperand(0); }

        Value *getDest() { return getOperand(1); }
    };

    // 这个指令比较重要，用于数组的访问，Base必须是一个指针（其实就是计算偏移量的，返回的也是指针）
    // Indices是一个vector, 第一维对应于指针Base，后面的维度对应于Base的维度
    // type* lhs = getelementptr type* base, {index1, index2, ...}
    struct GetElementPtrInstruction : public Instruction
    {
        GetElementPtrInstruction(pType type, Value *base, std::vector<Value *> indices, std::string name)
            : Instruction(type, MemoryOp::GEP)
        {
            setTotalUsers();
            addUse(base);
            for (auto index : indices)
                addUse(index);
            this->name = name;
        }

        bool isPointingConst();

        void emitIR(std::ostream &os) override;
        

        std::vector<Value *> getIndices()
        {
            std::vector<Value *> indices;
            for (int i = 1; i < (int)operands.size(); i++)
                indices.push_back(operands[i]);
            return indices;
        }

        static GetElementPtrInstruction *create(Value *base, std::vector<Value *> indices, std::string name = "");

        static GetElementPtrInstruction *create(pType type, Value *base, std::vector<Value *> indices, std::string name = "");
    };
        struct BranchInstruction : public Instruction
    {
        bool isCond = 0;
        bool isConditional() { return isCond; };
        bool isUnconditional() { return !isCond; };
        BranchInstruction(Value *condition, BasicBlock *trueBlock, BasicBlock *falseBlock);

        BranchInstruction(BasicBlock *thenBlock); // unconditional branch

        void setCondition(Value *condition);

        void setTrueBlock(BasicBlock *trueBlock);

        void setFalseBlock(BasicBlock *falseBlock);

        void setUnconditionalBlock(BasicBlock *thenBlock);

        Value *getCondition()
        {
            return operands[0];
        }

        BasicBlock *getTrueBlock()
        {
            return static_cast<BasicBlock *>(operands[1]);
        }

        BasicBlock *getFalseBlock()
        {
            return static_cast<BasicBlock *>(operands[2]);
        }

        BasicBlock *getUnconditionalBlock()
        {
            return static_cast<BasicBlock *>(operands[0]);
        }

        void emitIR(std::ostream &os) override
        {
            if (isCond)
            {
                os << "if" << " " << operands[0]->getTypeName() << " " << operands[0]->getIRName()
                   << ", goto label %" << operands[1]->getIRName()
                   << ", then label %" << operands[2]->getIRName() << std::endl;
            }
            else
            {
                os << "goto" << " label %" << operands[0]->getIRName() << std::endl;
            }
        }

        static BranchInstruction *createCondBr(Value *condition, BasicBlock *trueBlock, BasicBlock *falseBlock)
        {
            assert(condition->getType()->isIntegerTy());
            return new BranchInstruction(condition, trueBlock, falseBlock);
        }

        static BranchInstruction *createBr(BasicBlock *thenBlock)
        {
            return new BranchInstruction(thenBlock);
        }
    };

    struct ReturnInstruction : public Instruction
    {
        Function *parentFunc;
        ReturnInstruction(Value *retVal, Function *func);

        ReturnInstruction(Function *func);

        bool isVoidReturn()
        {
            return operands.size() == 0;
        }

        void emitIR(std::ostream &os) override
        {
            os << getOpStr();
            if (!isVoidReturn())
                os << " " << operands[0]->getTypeName() << " " << operands[0]->getIRName();
            else
                os << " void";
            os << std::endl;
        }

        static ReturnInstruction *create(Value *retVal, Function *func)
        {
            return new ReturnInstruction(retVal, func);
        }

        static ReturnInstruction *createVoid(Function *func)
        {
            return new ReturnInstruction(func);
        }
    };
        struct PhiInstruction : public Instruction
    {
        std::set<PVB> incomingValue;

        PhiInstruction(pType type, std::vector<PVB> value, std::string name = "");

        std::vector<PVB> getDifferentPVB();

        void emitIR(std::ostream &os) override;

        static PhiInstruction *create(pType type, std::vector<PVB> value, std::string name = "")
        {
            return new PhiInstruction(type, value, name);
        }

        int getValueBBSize() const
        {
            return operands.size() / 2;
        }

        PVB getValueBB(int i) const;

        std::vector<PVB> getIncomingValue() const;

        void insertIncomingValue(PVB value);

        bool allSameValue();

        void removeUseFromVector(Use *use) override;

        void removeEntry(BasicBlock *bb);
    };

    // 比较函数，没啥好测的，测一下emitIR，用ICmpInstruction和FCmpInstruction测试，使用静态函数创建对象
    struct CmpInstruction : public Instruction
    {
        const unsigned int cmpCode;
        CmpInstruction(CmpOp op, Value *lhs, Value *rhs);

        unsigned int getCmpCode() const
        {
            return cmpCode;
        }

        void emitIR(std::ostream &os) override
        {
            os << getIRName() << " = " << getOpStr() << " " << type->getTypeName() << " " << getOperand(0)->getIRName() << ", " << getOperand(1)->getIRName() << std::endl;
        }
    };

    struct ICmpInstruction : public CmpInstruction
    {
        ICmpInstruction(CmpOp cmp, Value *lhs, Value *rhs) : CmpInstruction(cmp, lhs, rhs) {}
        static ICmpInstruction *create(CmpOp cmp, Value *lhs, Value *rhs)
        {
            return new ICmpInstruction(cmp, lhs, rhs);
        }

        HANDLE_CMP_INST(ICmp, Gt)
        HANDLE_CMP_INST(ICmp, Ge)
        HANDLE_CMP_INST(ICmp, Lt)
        HANDLE_CMP_INST(ICmp, Le)
        HANDLE_CMP_INST(ICmp, Eq)
        HANDLE_CMP_INST(ICmp, Ne)

        // 创建一个新的ICmpInstruction，其比较结果与原ICmpInstruction相反
        static ICmpInstruction *createNotICmp(ICmpInstruction *icmp, bool resultNot = true)
        {
            auto cmp = icmp->getOpcode();
            auto newCmp = 0;
            switch (cmp)
            {
            case ICmpInstruction::Gt:
                newCmp = ICmpInstruction::Le;
                break;
            case ICmpInstruction::Ge:
                newCmp = ICmpInstruction::Lt;
                break;
            case ICmpInstruction::Lt:
                newCmp = ICmpInstruction::Ge;
                break;
            case ICmpInstruction::Le:
                newCmp = ICmpInstruction::Gt;
                break;
            case ICmpInstruction::Eq:
                newCmp = ICmpInstruction::Ne;
                break;
            case ICmpInstruction::Ne:
                newCmp = ICmpInstruction::Eq;
                break;
            default:
                break;
            }
            if (!resultNot)
                return ICmpInstruction::create((CmpOp)newCmp, icmp->getOperand(1), icmp->getOperand(0));
            else
                return ICmpInstruction::create((CmpOp)newCmp, icmp->getOperand(0), icmp->getOperand(1));
        }
    };

    struct FCmpInstruction : public CmpInstruction
    {
        FCmpInstruction(CmpOp cmp, Value *lhs, Value *rhs) : CmpInstruction(cmp, lhs, rhs) {}
        static FCmpInstruction *create(CmpOp cmp, Value *lhs, Value *rhs)
        {
            return new FCmpInstruction(cmp, lhs, rhs);
        }
        HANDLE_CMP_INST(FCmp, FGt)
        HANDLE_CMP_INST(FCmp, FGe)
        HANDLE_CMP_INST(FCmp, FLt)
        HANDLE_CMP_INST(FCmp, FLe)
        HANDLE_CMP_INST(FCmp, FEq)
        HANDLE_CMP_INST(FCmp, FNe)

        // 创建一个新的ICmpInstruction，其比较结果与原ICmpInstruction相反
        static FCmpInstruction *createNotFCmp(FCmpInstruction *icmp, bool resultNot = true)
        {
            auto cmp = icmp->getOpcode();
            auto newCmp = 0;
            switch (cmp)
            {
            case FCmpInstruction::FGt:
                newCmp = FCmpInstruction::FLe;
                break;
            case FCmpInstruction::FGe:
                newCmp = FCmpInstruction::FLt;
                break;
            case FCmpInstruction::FLt:
                newCmp = FCmpInstruction::FGe;
                break;
            case FCmpInstruction::FLe:
                newCmp = FCmpInstruction::FGt;
                break;
            case FCmpInstruction::FEq:
                newCmp = FCmpInstruction::FNe;
                break;
            case FCmpInstruction::FNe:
                newCmp = FCmpInstruction::FEq;
                break;
            default:
                break;
            }
            if (!resultNot)
                return FCmpInstruction::create((CmpOp)newCmp, icmp->getOperand(1), icmp->getOperand(0));
            else
                return FCmpInstruction::create((CmpOp)newCmp, icmp->getOperand(0), icmp->getOperand(1));
        };
    };

    struct CallInstruction : public Instruction
    {
        CallInstruction(Function *func, std::vector<Value *> args);
        void emitIR(std::ostream &os) override;

        static CallInstruction *create(Function *func, std::vector<Value *> args)
        {
            return new CallInstruction(func, args);
        }

        Function *getCallee()
        {
            assert(operands[0]->isFunction());
            return static_cast<Function *>(operands[0]);
        }

        std::vector<Value *> getArgs()
        {
            std::vector<Value *> args;
            for (int i = 1; i < (int)operands.size(); i++)
                args.push_back(operands[i]);
            return args;
        }

        bool isReturnVoid() { return type->isVoidTy(); }
    };
}