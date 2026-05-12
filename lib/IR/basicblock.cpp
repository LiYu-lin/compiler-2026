#include "basicblock.h"
#include "instruction.h"
#include "recycle.h"

namespace IR
{
    int BasicBlock::totBlocks = 0;

    void BasicBlock::waste()
    {
        parent->blocks().remove(this);
        ListNode *idx = instructions.begin();
        while (idx != instructions.end())
        {
            ListNode *nextInstr = idx->nextNode();
            static_cast<Instruction *>(idx)->waste();
            idx = nextInstr;
        }

        // 移除所有被使用的位�?
        for (ListNode *i = useList.begin(); i != useList.end(); i = i->nextNode())
        {
            Use *use = static_cast<Use *>(i);
            use->val->useList.remove(use);
            User *user = use->user;
            user->removeUseFromVector(use);
        }

        utils::Recycle::free(this, [](void *ptr)
                             { delete static_cast<BasicBlock *>(ptr); });
    }

    std::vector<Instruction *> BasicBlock::getVectorInstructions()
    {
        std::vector<Instruction *> instrs;
        for (ListNode *i = instructions.begin(); i != instructions.end(); i = i->nextNode())
        {
            instrs.push_back(static_cast<Instruction *>(i));
        }
        return instrs;
    }

    void BasicBlock::InsertInstructionBack(Instruction *instr)
    {
        instr->parent = this;
        instructions.pushBack(instr);
    }

    void BasicBlock::InsertInstructionFront(Instruction *instr)
    {
        instr->parent = this;
        instructions.pushFront(instr);
    }

    void BasicBlock::InsertInstruction(Instruction *instr, Instruction *who)
    {
        instr->parent = this;
        instructions.insertBefore(who, instr);
    }

    void BasicBlock::emitIR(std::ostream &os)
    {
        os << getIRName() << "(" + std::to_string(instructions.size()) + "):" << std::endl;

        if (!instructions.begin() || !instructions.end()) {
            std::cerr << "Error: Invalid instruction list in BasicBlock " << getIRName() << std::endl;
            return;
        }

        for (ListNode *i = instructions.begin(); i != instructions.end(); i = i->nextNode()) {
            if (!i) {  // 检查当前节点是否有�?
                std::cerr << "Warning: Null ListNode encountered" << std::endl;
                continue;
            }
            
            Instruction *idx = static_cast<Instruction *>(i);
            if (!idx) {  // 检查转换后的Instruction是否有效
                std::cerr << "Warning: Invalid Instruction pointer" << std::endl;
                continue;
            }
            
            os << '\t';
            idx->emitIR(os);
        }
    }

    void BasicBlock::emitUse(std::ostream &os)
    {
        for (ListNode *i = instructions.begin(); i != instructions.end(); i = i->nextNode())
        {
            Instruction *idx = static_cast<Instruction *>(i);
            idx->emitUse(os);
        }
    }

    std::set<BasicBlock *> BasicBlock::getSuccBlock()
    {
        std::set<BasicBlock *> res;
        // 如果不是跳转指令，那么后继节点就是下一个基本块
        if (!isReturnBlock() && !isCondBrBlock() && !isDirectBrBlock())
        {
            ListNode *nxt = nextNode();
            if (nxt && nxt->id != 0)
            {
                auto temp = static_cast<BasicBlock *>(nxt);
                res.insert(temp);
            }
        }
        else if (isReturnBlock())
        {
            // 如果是return block，那么没有后�?
        }
        else if (isCondBrBlock())
        {
            // 如果是条件跳转块，那么后继是条件跳转的两个块
            auto instr = static_cast<BranchInstruction *>(instructions.back());
            res.insert(instr->getTrueBlock());
            res.insert(instr->getFalseBlock());
        }
        else if (isDirectBrBlock())
        {
            // 如果是无条件跳转块，那么后继是无条件跳转的块
            auto instr = static_cast<BranchInstruction *>(instructions.back());
            res.insert(instr->getUnconditionalBlock());
        }
        return res;
    }

    std::set<BasicBlock *> BasicBlock::getPredBlock()
    {
        std::set<BasicBlock *> res;
        auto myUses = getVectorUses();
        for (Use *use : myUses)
        {
            User *user = use->user;
            if (user->isInstruction())
            {
                Instruction *instr = static_cast<Instruction *>(user);
                if (instr->getOpcode() == Instruction::BR)
                {
                    res.insert(instr->getParentBB());
                }
            }
        }

        ListNode *prev = prevNode();
        if (prev && prev->id != 0)
        {
            BasicBlock *prevBB = static_cast<BasicBlock *>(prev);
            if (!prevBB->isReturnBlock() && !prevBB->isCondBrBlock() && !prevBB->isDirectBrBlock())
            {
                res.insert(prevBB);
            }
        }
        return res;
    }

    bool BasicBlock::checkNeighbour(BasicBlock *a, BasicBlock *b)
    {
        if (a->nextNode() != b)
            return false;
        if (a->isCondBrBlock() || a->isDirectBrBlock() || a->isReturnBlock())
            return false;
        if (!b->getInstruction().empty())
        {
            auto first = b->getInstruction().begin();
            if (static_cast<Instruction *>(first)->getOpcode() == Instruction::Phi)
                return false;
        }
        auto aSucc = a->getSuccBlock();
        auto bPred = b->getPredBlock();
        if (aSucc.size() != 1 || bPred.size() != 1)
            return false;
        return *aSucc.begin() == b && *bPred.begin() == a;
    }

    bool BasicBlock::isReturnBlock()
    {
        if (instructions.empty())
            return false;
        ListNode *last = instructions.back();
        return static_cast<Instruction *>(last)->getOpcode() == Instruction::Return;
    }

    bool BasicBlock::isCondBrBlock()
    {
        if (instructions.empty())
            return false;
        ListNode *last = instructions.back();
        return static_cast<Instruction *>(last)->getOpcode() == Instruction::BR &&
               static_cast<BranchInstruction *>(last)->isConditional();
    }

    bool BasicBlock::isDirectBrBlock()
    {
        if (instructions.empty())
            return false;
        ListNode *last = instructions.back();
        return static_cast<Instruction *>(last)->getOpcode() == Instruction::BR &&
               static_cast<BranchInstruction *>(last)->isUnconditional();
    }

    void BasicBlock::emitSuss(std::ostream &os)
    {
        os << '\t' << getIRName() << " successors: \n";
        for (auto v : getSuccBlock())
            os << '\t' << v->getIRName() << " ";
        os << "\tend" << std::endl;
    }

    BasicBlock *BasicBlock::get(std::string name)
    {
        return new BasicBlock(name);
    }

    void BasicBlock::removeEntryFromPhi(BasicBlock *toRemoveBlock)
    {
        for (Instruction *instr : getVectorInstructions())
        {
            if (instr->getOpcode() == Instruction::Phi)
            {
                PhiInstruction *phiInstr = static_cast<PhiInstruction *>(instr);
                phiInstr->removeEntry(toRemoveBlock);
            }
            else
                break;
        }
    }

} // namespace IR
