#pragma once

#include "Pass.h"

namespace IR {

class SimplifyCFG : public ModulePass {
public:
    explicit SimplifyCFG(Module& module) : ModulePass(module) {}

    std::string name() const override { return "simplify-cfg"; }
    bool run() override;
    std::map<std::string, int> stats() const override;

private:
    int foldedBranches = 0;
    int removedBlocks = 0;
};

}  // namespace IR
