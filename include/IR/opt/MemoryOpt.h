#pragma once

#include "ir/opt/Pass.h"

namespace IR {

class MemoryOpt : public ModulePass {
public:
    explicit MemoryOpt(Module& module) : ModulePass(module) {}

    std::string name() const override { return "memory-opt"; }
    bool run() override;
    std::map<std::string, int> stats() const override;

private:
    int forwardedLoads = 0;
    int removedStores = 0;
};

}  // namespace IR
