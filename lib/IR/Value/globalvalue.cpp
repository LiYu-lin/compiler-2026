#include "ir/Value/globalvalue.h"
#include "ir/type.h"
#include "ir/basicblock.h"
#include "ir/Value/instruction.h"
namespace IR
{
    Function::Function(pType retType, std::string name)
        : GlobalValue(nullptr, name, Value::FunctionVal)
    {
        this->type = FunctionType::getFunctionType(retType,{});
    }

    Function::Function(pType retType, std::string name, std::vector<pType > argtypes)
        : GlobalValue(nullptr, name, Value::FunctionVal)
    {
        this->type = FunctionType::getFunctionType(retType, std::move(argtypes));
        for (unsigned int i = 0; i < argtypes.size(); i++)
        {
            pType argtype = argtypes[i];
            funArgs.push_back(new Argument(argtype, i));
        }
    }

    std::vector<BasicBlock *> Function::getVectorBlocks()
    {
        std::vector<BasicBlock *> blocks;
        for (ListNode *i = funBlocks.getHead(); i != funBlocks.getTail(); i = i->getNext())
        {
            blocks.push_back(static_cast<BasicBlock *>(i));
        }
        return blocks;
    }

    void Function::waste()
    {
        for (ListNode *i = funBlocks.getHead(); i != funBlocks.getTail(); i = i->getNext())
        {
            BasicBlock *idx = static_cast<BasicBlock *>(i);
            idx->waste();
        }
    }

    void Function::emitIR(std::ostream &os)
    {
        os << "define " << type->getTypeName() << " " << getIRName() << " {\n";
        for (ListNode *i = funBlocks.getHead(); i != funBlocks.getTail(); i = i->getNext())
        {
            BasicBlock *BB = static_cast<BasicBlock *>(i);
            if (!BB) { 
            std::cerr << "Warning: Null BasicBlock encountered" << std::endl;
            continue;
        }
            BB->emitIR(os);
        }
        os << "}\n";
    }

    void Function::addBlock(BasicBlock *block, bool entry)
    {
        // 更新尾节点的 susccsor, （不需要了）
        block->parent = this;
        funBlocks.PushBack(block);
        if (funBlocks.getHead() == block)
            entryBlock = block;
        if (entry)
            entryBlock = block;
    }

    void Function::insertBlock(BasicBlock *block, BasicBlock *prev)
    {
        // 首先先更新前一个块的后继。（不需要了）
        block->parent = this;
        funBlocks.insertAfter(prev, block);

        // 更新 block 的后继。（不需要了）
    }

    void Function::removeBlock(BasicBlock *block)
    {
        block->waste();
    }

    void Function::emitUse(std::ostream &os)
    {
        os << getIRName() << " used by:" << std::endl;
        for (ListNode *i = useList.getHead(); i != useList.getTail(); i = i->getNext())
        {
            Use *use = static_cast<Use *>(i);
            os << '\t' << use->val->getIRName() << std::endl;
        }
        for (ListNode *i = funBlocks.getHead(); i != funBlocks.getTail(); i = i->getNext())
        {
            BasicBlock *idx = static_cast<BasicBlock *>(i);
            idx->emitUse(os);
        }
    }

    std::map<BasicBlock *, std::set<BasicBlock *>> Function::cfg()
    {
        std::map<BasicBlock *, std::set<BasicBlock *>> cfg;
        if (isBuiltinFunction())
            return cfg;
        for (ListNode *i = funBlocks.getHead(); i != funBlocks.getTail(); i = i->getNext())
        {
            BasicBlock *BB = static_cast<BasicBlock *>(i);
            auto succs = BB->getSuccBlock();
            cfg[BB] = std::set<BasicBlock *>();
            for (auto &succ : succs)
                cfg[BB].insert(succ);
        }

        return cfg;
    }

    void Function::mergeBlock(BasicBlock *prev, BasicBlock *nxt)
    {
        assert(BasicBlock::checkNeignbour(prev, nxt));

        nxt->replaceAllUsageTo(prev);
        // 更新 prev 的指令
        for (ListNode *i = nxt->getInstruction().getHead(); i != nxt->getInstruction().getTail();)
        {
            Instruction *instr = static_cast<Instruction *>(i);
            i = i->getNext();
            nxt->getInstruction().remove(instr);
            prev->InsertInstructionBack(instr);
        }
        assert(nxt->instructions.isEmpty());
        nxt->waste();
    }

    BasicBlock *Function::splitBlock(BasicBlock *block, Instruction *pos)
    {
        for (ListNode *i = block->getInstruction().getHead(); i != block->getInstruction().getTail(); i = i->getNext())
        {
            Instruction *idx = static_cast<Instruction *>(i);
            if (idx == pos)
            {
                BasicBlock *newBlock = new BasicBlock("split");
                insertBlock(newBlock, block);
                for (ListNode *i = pos->getNext(); i != block->getInstruction().getTail();)
                {
                    Instruction *instr = static_cast<Instruction *>(i);
                    i = i->getNext();
                    block->getInstruction().remove(instr);
                    newBlock->InsertInstructionBack(instr);
                }
                return newBlock;
            }
        }
        return nullptr;
    }

    ExternalFunction::ExternalFunction(pType retType, std::string name, std::vector<pType > argtypes)
        : Function(retType, name, argtypes)
    {
        type = FunctionType::getFunctionType(retType, std::move(argtypes));
        isBuiltin = true;
    }

    void ExternalFunction::emitIR(std::ostream &os)
    {
        os << "declare " << type->getTypeName() << " " << getIRName() << std::endl;
    }

    GlobalVariable::GlobalVariable(pType type, std::string name, bool isConstant)
        : GlobalValue(type, name, Value::GlobalVariableVal)
    {
        this->isConstant = isConstant;
        operands.push_back(Constant::getZeroValueForType(type->getBase()));
    }

    GlobalVariable::GlobalVariable(pType type, std::string name, Constant *initializer, bool isConstant) : GlobalValue(type, name, Value::GlobalVariableVal)
    {
        isInit = true;
        this->initializer = initializer;
        this->isConstant = isConstant;
        operands.push_back(initializer);
    }

    void GlobalVariable::emitIR(std::ostream &os)
    {
        os << "define global " << type->getBase()->getTypeName()
           << " " << getIRName() << std::endl;
    }
}