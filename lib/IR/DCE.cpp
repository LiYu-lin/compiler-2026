#include "DCE.h"
#include "instruction.h"

namespace IR {

bool DCE::run() {
    removedInstructions = 0;
    bool changed = false;
    bool localChanged = true;

    while (localChanged) {
        localChanged = false;

        for (auto *function : module.getFunctionList()) {
            if (!function || function->isBuiltinFunction()) {
                continue;
            }

            for (auto *block : function->getVectorBlocks()) {
                auto instructions = block->getVectorInstructions();
                for (auto *instruction : instructions) {
                    if (!instruction || !instruction->isUseless()) {
                        continue;
                    }

                    instruction->waste();
                    ++removedInstructions;
                    localChanged = true;
                    changed = true;
                }
            }
        }
    }

    return changed;
}

std::map<std::string, int> DCE::stats() const {
    return {{"removed_instructions", removedInstructions}};
}

}  // namespace IR
