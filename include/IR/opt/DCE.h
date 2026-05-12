#pragma once

#include "Pass.h"

namespace IR {

class DCE : public ModulePass {
public:
    explicit DCE(Module& module) : ModulePass(module) {}

    std::string name() const override { return "dce"; }
    bool run() override;
    std::map<std::string, int> stats() const override;

private:
    int removedInstructions = 0;
};

}  // namespace IR
