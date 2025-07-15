#include "ir/Value/instruction.h"
#include "ir/basicblock.h"
#include "ir/type.h"
#include "utils/error.h"
#include "utils/recycle.h"
#include <algorithm>
namespace IR {
     UnaryInstruction::UnaryInstruction(pType type, const unsigned int x, Value *op) : Instruction(type, x)
    {
        setTotalUsers();
        addUse(op);
    }
    UnaryInstruction::UnaryInstruction(pType type, UnaryOp x, Value *op) : Instruction(type, x)
    {
        setTotalUsers();
        addUse(op);
    }

    CastInstruction::CastInstruction(pType type, CastOp x, Value *op) : UnaryInstruction(type, x, op) {}
    CastInstruction::CastInstruction(pType type, const unsigned int x, Value *op) : UnaryInstruction(type, x, op) {}
        BranchInstruction::BranchInstruction(Value *condition, BasicBlock *trueBlock, BasicBlock *falseBlock)
        : Instruction(Type ::getVoidType(), TerminatorOp::BR)
    {
        setTotalUsers();
        addUse(condition);
        addUse(trueBlock);
        addUse(falseBlock);
        isCond = true;
    }

    BranchInstruction::BranchInstruction(BasicBlock *thenBlock) // unconditional branch
        : Instruction(Type ::getVoidType(), TerminatorOp::BR)
    {
        setTotalUsers();
        addUse(thenBlock);
        isCond = false;
    }

    void BranchInstruction::setCondition(Value *condition)
    {
        if (!isCond)
            Error::Error(__PRETTY_FUNCTION__, "Branch is not conditional");
        auto use = uses[0];
        use->setValue(condition);
        assert(operands[0] == condition);
    }

    void BranchInstruction::setTrueBlock(BasicBlock *trueBlock)
    {
        if (!isCond)
            Error::Error(__PRETTY_FUNCTION__, "Branch is not conditional");
        auto use = uses[1];
        use->setValue(trueBlock);
        assert(operands[1] == trueBlock);
    }

    void BranchInstruction::setFalseBlock(BasicBlock *falseBlock)
    {
        if (!isCond)
            Error::Error(__PRETTY_FUNCTION__, "Branch is not conditional");
        auto use = uses[2];
        use->setValue(falseBlock);
        assert(operands[2] == falseBlock);
    }

    void BranchInstruction::setUnconditionalBlock(BasicBlock *thenBlock)
    {
        if (isCond)
            Error::Error(__PRETTY_FUNCTION__, "Branch is not unconditional");
        auto use = uses[0];
        use->setValue(thenBlock);
        assert(operands[0] == thenBlock);
    }

    ReturnInstruction::ReturnInstruction(Value *retVal, Function *func)
        : Instruction(Type ::getVoidType(), TerminatorOp::Return)
    {
        setTotalUsers();
        addUse(retVal);
        parentFunc = func;
    }

    ReturnInstruction::ReturnInstruction(Function *func)
        : Instruction(Type ::getVoidType(), TerminatorOp::Return)
    {
        setTotalUsers();
        parentFunc = func;
    }
    BinaryInstruction::BinaryInstruction(pType ty, BinaryOp op, Value *lhser, Value *rhser): Instruction(ty, op)
    {
        setTotalUsers();
        if (!checkType(lhser->getType(), rhser->getType()))
        {
            Error::Error(__PRETTY_FUNCTION__, "Binary operands must have the same type");
        }
        addUse(lhser);
        addUse(rhser);
    }
        AllocaInstruction::AllocaInstruction(pType ty, std::string name, bool isConst)
        : Instruction(ty, MemoryOp::Alloca)
    {
        setTotalUsers();
        this->name = name;
        this->isConst = isConst;
        if (isConst)
            operands.push_back(Constant::getZeroValueForType(ty->getBase()));
    }

    AllocaInstruction::AllocaInstruction(pPointerType type, std::string name, Constant *init, bool isConst)
        : Instruction(type, MemoryOp::Alloca)
    {
        setTotalUsers();
        this->name = name;
        this->isConst = isConst;
        operands.push_back(init);
    }

    StoreInstruction::StoreInstruction(Value *lhs, Value *dest) : Instruction(Type ::getVoidType(), MemoryOp::Store)
    {
        setTotalUsers();
        if (!checkType(lhs->getType(), dest->getType()->getBase()))
        {
            Error::Error(__PRETTY_FUNCTION__, "Binary operands must have the same type");
        }
        addUse(lhs);
        addUse(dest);
    }

    bool GetElementPtrInstruction::isPointingConst()
    {
        if (!type->getBase()->isFloatTy() && !type->getBase()->isInt32Ty())
            return false;
        auto base = operands[0];
        if (base->isGlobalVariable() && static_cast<GlobalVariable *>(base)->isPointingConst())
        {
            goto label;
        }
        else if (base->isInstruction() &&
                 static_cast<Instruction *>(base)->getOpcode() == Instruction::Alloca &&
                 static_cast<AllocaInstruction *>(base)->isPointingConst())
            goto label;
        else if (base->isInstruction() &&
                 static_cast<Instruction *>(base)->getOpcode() == Instruction::GEP &&
                 static_cast<GetElementPtrInstruction *>(base)->isPointingConst())
            goto label;
        return false;
    label:
        for (int i = 1; i < (int)operands.size(); i++)
        {
            if (!operands[i]->isConstantInt32())
                return false;
        }
        return true;
    }

    void GetElementPtrInstruction::emitIR(std::ostream &os)
    {
        std::string lhs = getIRName();
        std::string base = getOperand(0)->getTypeName() + " " + getOperand(0)->getIRName();
        std::string indices = "";
        int m = operands.size();
        for (int i = 1; i < m; i++)
        {
            indices += getOperand(i)->getTypeName() + " " + getOperand(i)->getIRName();
            if (i != m - 1)
                indices += ", ";
        }
        os << lhs << " = " << getOpStr() << " " << getTypeName() << " " << base << ", " << indices << std::endl;
    }

    GetElementPtrInstruction *IR::GetElementPtrInstruction::create(Value *base, std::vector<Value *> indices, std::string name)
    {
        if (!base->getType()->isPointerTy())
            Error::Error(__PRETTY_FUNCTION__, "Operand must be a pointer type");
        if (indices.size() == 0)
            Error::Error(__PRETTY_FUNCTION__, "Indices must not be empty");

        pType geptype = base->getType();
        for (int i = 0; i < (int)indices.size(); i++)
            geptype = geptype->getBase();
        return new GetElementPtrInstruction(PointerType::getPointerType(geptype), base, indices, name);
    }

    GetElementPtrInstruction *IR::GetElementPtrInstruction::create(pType type, Value *base, std::vector<Value *> indices, std::string name)
    {
        if (!base->getType()->isPointerTy())
            Error::Error(__PRETTY_FUNCTION__, "Operand must be a pointer type");
        if (indices.size() == 0)
            Error::Error(__PRETTY_FUNCTION__, "Indices must not be empty");

        return new GetElementPtrInstruction(type, base, indices, name);
    }
    PhiInstruction::PhiInstruction(pType type, std::vector<PVB> value, std::string name)
        : Instruction(type, OtherOp::Phi)
    {
        setTotalUsers();
        for (auto p : value)
        {
            addUse(p.first);
            assert(p.second->isBasicBlock());
            addUse(p.second);
        }
    }

    std::vector<PVB> PhiInstruction::getIncomingValue() const
    {
        std::vector<PVB> res;
        for (int i = 0; i < (int)operands.size() / 2; i++)
        {
            res.push_back(getValueBB(i));
        }
        return res;
    }

    PVB PhiInstruction::getValueBB(int i) const
    {
        assert(i < (int)operands.size() / 2);
        assert(operands[i * 2 + 1]->isBasicBlock());
        return std::make_pair(operands[i * 2],
                              static_cast<BasicBlock *>(operands[i * 2 + 1]));
    }

    void PhiInstruction::emitIR(std::ostream &os)
    {
        std::string lhs = getIRName();
        std::string type = getTypeName();
        os << lhs << " = " << "phi " << type << " ";
        int PVBSize = operands.size() / 2;
        assert(operands.size() % 2 == 0);
        for (int i = 0; i < PVBSize; i++)
        {
            auto [val, bb] = getValueBB(i);
            assert(bb->isBasicBlock());
            os << "[ " << val->getIRName()
               << ", " << bb->getIRName() << " ]";
            if (i != PVBSize - 1)
                os << ", ";
        }
        os << std::endl;
    }

    void PhiInstruction::insertIncomingValue(PVB value)
    {
        addUse(value.first);
        addUse(value.second);
        assert(operands.back()->isBasicBlock());
    }

    bool PhiInstruction::allSameValue()
    {
        std::set<PVB> temp;
        for (int i = 0; i < (int)operands.size() / 2; i++)
            temp.insert(getValueBB(i));
        return temp.size() == 1;
    }

    void PhiInstruction::removeUseFromVector(Use *use)
    {
        if (use->val->isBasicBlock())
        {
            auto iterator = std::find(operands.begin(), operands.end(), use->val);
            if (iterator == operands.end())
                return;
            int it = iterator - operands.begin();
            assert(it % 2 == 1);
            uses[it - 1]->val->useList.remove(uses[it - 1]);
            uses[it]->val->useList.remove(uses[it]);
            operands.erase(operands.begin() + it - 1, operands.begin() + it + 1);
            uses.erase(uses.begin() + it - 1, uses.begin() + it + 1);
        }
        else
        {
            auto iterator = std::find(operands.begin(), operands.end(), use->val);
            if (iterator == operands.end())
                return;
            int it = iterator - operands.begin();
            assert(it % 2 == 0);
            uses[it]->val->useList.remove(uses[it]);
            uses[it + 1]->val->useList.remove(uses[it + 1]);
            operands.erase(operands.begin() + it, operands.begin() + it + 2);
            uses.erase(uses.begin() + it, uses.begin() + it + 2);
        }
    }

    std::vector<PVB> PhiInstruction::getDifferentPVB()
    {
        std::vector<PVB> res;
        std::set<PVB> temp;
        for (int i = 0; i < (int)operands.size() / 2; i++)
            temp.insert(getValueBB(i));
        for (auto p : temp)
            res.push_back(p);
        return res;
    }

    void PhiInstruction::removeEntry(BasicBlock *bb)
    {
        for (int i = 0; i < (int)operands.size() / 2; i++)
        {
            if (operands[i * 2 + 1] == bb)
            {
                uses[i * 2]->val->useList.remove(uses[i * 2]);
                uses[i * 2 + 1]->val->useList.remove(uses[i * 2 + 1]);
                operands.erase(operands.begin() + i * 2, operands.begin() + i * 2 + 2);
                uses.erase(uses.begin() + i * 2, uses.begin() + i * 2 + 2);
                i--;
            }
        }
    }

    // CmpInstruction
    CmpInstruction::CmpInstruction(CmpOp op, Value *lhs, Value *rhs)
        : Instruction(Type::getI32Type(), op), cmpCode(op)
    {
        setTotalUsers();
        // if (!Type::checkType(lhs->getType(), rhs->getType()))
        // {
        //     Error::Error(__PRETTY_FUNCTION__, "Binary operands must have the same type");
        // }
        addUse(lhs);
        addUse(rhs);
    }

    CallInstruction::CallInstruction(Function *func, std::vector<Value *> args) : Instruction(func->getReturnType(), OtherOp::Call)
    {
        if (!type->isVoidTy())
            setTotalUsers();
        addUse(func);
        for (auto arg : args)
        {
            addUse(arg);
        }
    }

    void CallInstruction::emitIR(std::ostream &os)
    {
        if (!type->isVoidTy())
            os << getIRName() << " = call ";
        else
            os << "call ";
        os << type->getTypeName() << " " << operands[0]->getIRName() << "(";
        int m = operands.size();
        for (int i = 1; i < m; i++)
        {
            os << operands[i]->getType()->getTypeName() << " " << operands[i]->getIRName();
            if (i != m - 1)
            {
                os << ", ";
            }
        }
        os << ")" << std::endl;
    }


    std::string Instruction::getOpStr()
    {
        switch (getOpcode())
        {
        case Add:
            return "add";
        case Sub:
            return "sub";
        case Mul:
            return "mul";
        case Div:
            return "sdiv";
        case Rem:
            return "srem";
        case FAdd:
            return "fadd";
        case FSub:
            return "fsub";
        case FMul:
            return "fmul";
        case FDiv:
            return "fdiv";
        case FRem:
            return "frem";
        case And:
            return "and";
        case Or:
            return "or";
        case Xor:
            return "xor";
        case Neg:
            return "neg";
        case FNeg:
            return "fneg";
        case Not:
            return "not";
        case Alloca:
            return "alloca";
        case Load:
            return "load";
        case Store:
            return "store";
        case GEP:
            return "getElementPtr";
        case FPtoSI:
            return "fptosi";
        case SItoFP:
            return "sitofp";
        case ICmp:
            return "icmp";
        case FCmp:
            return "fcmp";
        case Call:
            return "call";
        case Phi:
            return "phi";
        case Select:
            return "select";
        case BR:
            return "br";
        case Ne:
            return "ne";
        case Lt:
            return "lt";
        case Le:
            return "le";
        case Gt:
            return "gt";
        case Ge:
            return "ge";
        case Eq:
            return "eq";
        case FNe:
            return "fne";
        case FLt:
            return "flt";
        case FLe:
            return "fle";
        case FGt:
            return "fgt";
        case FGe:
            return "fge";
        case FEq:
            return "feq";
        case Return:
            return "return";
        default:
            return "unknown";
        }
    }

    void Instruction::remove() {
        // 从BB中的instructionList中移除
        if (prev != nullptr)
        {
            prev->next = next;
        }
        if (next != nullptr)
        {
            next->prev = prev;
        }
    }

    void Instruction::waste()
    {
        // 从BB中的instructionList中移除
        assert(parent != nullptr); // 保证parent不为空
        parent->getInstruction().remove(this);

        // 移除所有被使用的位置
        for (ListNode* i = useList.getHead(); i != useList.getTail(); i = i->getNext())
        {
            Use* use = static_cast<Use*>(i);
            use->val->useList.remove(use);
            User* user = use->user;
            user->removeUseFromVector(use);
        }

        // 移除所有的使用, 将其从对应value的useList中删除
        for (auto &use : uses)
        {
            use->val->useList.remove(use);
        }

        utils::Recycle::free(this, [](void *ptr) {
            delete static_cast<Instruction *>(ptr);
        });
    }

    bool Instruction::isUseless()
    {
        switch (getOpcode())
        {
        case Store:
            return false;
        case Call:
            return false;
        case Return:
            return false;
        case BR:
            return false;
        case InderectBr:
            return false;
        default:
            return useList.isEmpty();
        }
    }

} // namespace IR


