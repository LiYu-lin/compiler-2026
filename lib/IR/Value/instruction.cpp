#include "ir/Value/instruction.h"
#include "ir/basicblock.h"
#include "ir/type.h"

namespace IR {
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


} // namespace IR


