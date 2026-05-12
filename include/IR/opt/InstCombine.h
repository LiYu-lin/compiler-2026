#pragma once

#include "ir/opt/Pass.h"

namespace IR {

class InstCombine : public ModulePass {
public:
    explicit InstCombine(Module& module) : ModulePass(module) {}

    std::string name() const override { return "instcombine"; }
    bool run() override;
    std::map<std::string, int> stats() const override;

private:
    int foldedInstructions = 0;
};

}  // namespace IR
