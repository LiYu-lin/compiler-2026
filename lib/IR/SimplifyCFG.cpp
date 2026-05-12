#include "SimplifyCFG.h"
#include "constant.h"
#include "globalvalue.h"
#include "instruction.h"

#include <set>
#include <vector>

namespace IR {

namespace {

std::vector<BasicBlock*> collectBlocks(Function* function) {
    std::vector<BasicBlock*> blocks;
    if (!function || function->isBuiltinFunction()) {
        return blocks;
    }

    for (ListNode* node = function->blocks().begin(); node != function->blocks().end();
         node = node->nextNode()) {
        blocks.push_back(static_cast<BasicBlock*>(node));
    }
    return blocks;
}

void markReachable(BasicBlock* block, std::set<BasicBlock*>& reachable) {
    if (!block || reachable.count(block)) {
        return;
    }

    reachable.insert(block);
    for (auto* succ : block->getSuccBlock()) {
        markReachable(succ, reachable);
    }
}

}  // namespace

bool SimplifyCFG::run() {
    foldedBranches = 0;
    removedBlocks = 0;
    bool changed = false;

    for (auto* function : module.getFunctionList()) {
        if (!function || function->isBuiltinFunction()) {
            continue;
        }

        for (auto* block : collectBlocks(function)) {
            if (!block || !block->isCondBrBlock()) {
                continue;
            }

            auto* branch = static_cast<BranchInstruction*>(block->getInstruction().back());
            auto* condition = branch->getCondition();
            if (!condition || !condition->isConstantInt32()) {
                continue;
            }

            auto* selected = static_cast<ConstantInt32*>(condition)->getValue() != 0
                                 ? branch->getTrueBlock()
                                 : branch->getFalseBlock();
            branch->waste();
            block->InsertInstructionBack(BranchInstruction::createBr(selected));
            ++foldedBranches;
            changed = true;
        }

        std::set<BasicBlock*> reachable;
        markReachable(function->getEntryBlock(), reachable);

        auto blocks = collectBlocks(function);
        for (auto* block : blocks) {
            if (!block || block == function->getEntryBlock() || reachable.count(block)) {
                continue;
            }

            for (auto* succ : block->getSuccBlock()) {
                succ->removeEntryFromPhi(block);
            }
            block->waste();
            ++removedBlocks;
            changed = true;
        }
    }

    return changed;
}

std::map<std::string, int> SimplifyCFG::stats() const {
    return {{"folded_branches", foldedBranches}, {"removed_blocks", removedBlocks}};
}

}  // namespace IR
