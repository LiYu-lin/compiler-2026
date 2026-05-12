#include "MemoryOpt.h"
#include "instruction.h"

#include <unordered_map>

namespace IR {

namespace {

bool hasMemoryClobberBetween(Instruction* first, Instruction* last) {
    for (auto* node = first->nextNode(); node && node->id != 0 && node != last;
         node = node->nextNode()) {
        auto* instruction = static_cast<Instruction*>(node);
        if (instruction->getOpcode() == Instruction::Store ||
            instruction->getOpcode() == Instruction::Call) {
            return true;
        }
    }
    return false;
}

}  // namespace

bool MemoryOpt::run() {
    forwardedLoads = 0;
    removedStores = 0;
    bool changed = false;

    for (auto* function : module.getFunctionList()) {
        if (!function || function->isBuiltinFunction()) {
            continue;
        }

        for (auto* block : function->getVectorBlocks()) {
            std::unordered_map<Value*, Value*> knownMemory;
            auto instructions = block->getVectorInstructions();

            for (auto* instruction : instructions) {
                if (!instruction) {
                    continue;
                }

                switch (instruction->getOpcode()) {
                    case Instruction::Load: {
                        auto* load = static_cast<LoadInstruction*>(instruction);
                        auto found = knownMemory.find(load->getSrc());
                        if (found == knownMemory.end()) {
                            break;
                        }

                        load->replaceAllUsageTo(found->second);
                        load->waste();
                        ++forwardedLoads;
                        changed = true;
                        break;
                    }
                    case Instruction::Store: {
                        auto* store = static_cast<StoreInstruction*>(instruction);
                        auto* dest = store->getDest();
                        auto* src = store->getSrc();
                        if (src && src->isInstruction()) {
                            auto* srcInst = static_cast<Instruction*>(src);
                            if (srcInst->getOpcode() == Instruction::Load &&
                                static_cast<LoadInstruction*>(srcInst)->getSrc() == dest &&
                                srcInst->getParentBB() == block &&
                                !hasMemoryClobberBetween(srcInst, store)) {
                                store->waste();
                                ++removedStores;
                                changed = true;
                                break;
                            }
                        }

                        auto found = knownMemory.find(dest);
                        if (found != knownMemory.end() && found->second == src) {
                            store->waste();
                            ++removedStores;
                            changed = true;
                            break;
                        }

                        knownMemory[dest] = src;
                        break;
                    }
                    case Instruction::Call:
                        // Calls may read or write memory through globals or pointer arguments.
                        knownMemory.clear();
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return changed;
}

std::map<std::string, int> MemoryOpt::stats() const {
    return {{"forwarded_loads", forwardedLoads}, {"removed_stores", removedStores}};
}

}  // namespace IR
