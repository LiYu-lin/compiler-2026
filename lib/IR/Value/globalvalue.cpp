#include "globalvalue.h"
#include "type.h"
#include "basicblock.h"
#include "instruction.h"
#include <iostream>
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
        auto params = argtypes;
        this->type = FunctionType::getFunctionType(retType, std::move(params));
        for (unsigned int i = 0; i < argtypes.size(); i++)
        {
            pType argtype = argtypes[i];
            auto *arg = new Argument(argtype, i);
            arg->parent = this;
            funArgs.push_back(arg);
        }
    }

    std::vector<BasicBlock *> Function::getVectorBlocks()
    {
        std::vector<BasicBlock *> blocks;
        for (ListNode *i = funBlocks.begin(); i != funBlocks.end(); i = i->nextNode())
        {
            blocks.push_back(static_cast<BasicBlock *>(i));
        }
        return blocks;
    }

    void Function::waste()
    {
        for (ListNode *i = funBlocks.begin(); i != funBlocks.end();)
        {
            BasicBlock *idx = static_cast<BasicBlock *>(i);
            i = i->nextNode();
            idx->waste();
        }
    }

    void Function::emitIR(std::ostream &os)
    {
        os << "define " << type->getTypeName() << " " << getIRName() << " {\n";
        auto blocks = getVectorBlocks();
    for (ListNode* i = funBlocks.begin(); i != funBlocks.end(); i = i->nextNode()) {
        BasicBlock* BB = static_cast<BasicBlock*>(i);
        if (!BB) {
            std::cerr << "Warning: Null BasicBlock encountered in function " 
                    << getIRName() << std::endl;
            continue;
        }
        
        BB->emitIR(os);
    }
        os << "}\n";
    }

    void Function::addBlock(BasicBlock *block, bool entry)
    {
        block->parent = this;
        funBlocks.pushBack(block);
        if (funBlocks.begin() == block){
            entryBlock = block;
        }

        if (entry){
            entryBlock = block;
        }
            
    }

    void Function::insertBlock(BasicBlock *block, BasicBlock *prev)
    {
        // 首先先更新前一个块的后继。（不需要了�?
        block->parent = this;
        funBlocks.insertAfter(prev, block);

        // 更新 block 的后继。（不需要了�?
    }

    void Function::removeBlock(BasicBlock *block)
    {
        block->waste();
    }

    void Function::emitUse(std::ostream &os)
    {
        os << getIRName() << " used by:" << std::endl;
        for (ListNode *i = useList.begin(); i != useList.back(); i = i->nextNode())
        {
            Use *use = static_cast<Use *>(i);
            os << '\t' << use->val->getIRName() << std::endl;
        }
        for (ListNode *i = funBlocks.begin(); i != funBlocks.end(); i = i->nextNode())
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
        for (ListNode *i = funBlocks.begin(); i != funBlocks.end(); i = i->nextNode())
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
        assert(BasicBlock::checkNeighbour(prev, nxt));

        nxt->replaceAllUsageTo(prev);
        // 更新 prev 的指�?
        for (ListNode *i = nxt->getInstruction().begin(); i != nxt->getInstruction().back();)
        {
            Instruction *instr = static_cast<Instruction *>(i);
            i = i->nextNode();
            nxt->getInstruction().remove(instr);
            prev->InsertInstructionBack(instr);
        }
        assert(nxt->instructions.empty());
        nxt->waste();
    }

    BasicBlock *Function::splitBlock(BasicBlock *block, Instruction *pos)
    {
        for (ListNode *i = block->getInstruction().begin(); i != block->getInstruction().back(); i = i->nextNode())
        {
            Instruction *idx = static_cast<Instruction *>(i);
            if (idx == pos)
            {
                BasicBlock *newBlock = new BasicBlock("split");
                insertBlock(newBlock, block);
                for (ListNode *i = pos->nextNode(); i != block->getInstruction().back();)
                {
                    Instruction *instr = static_cast<Instruction *>(i);
                    i = i->nextNode();
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
        os << "define global " << getTypeName()
           << " " << getIRName() << std::endl;
    }
}
