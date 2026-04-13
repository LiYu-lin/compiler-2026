#pragma once

#include "ir/opt/Pass.h"

namespace IR {

class SimplifyCFG : public ModulePass {
public:
    explicit SimplifyCFG(Module& module) : ModulePass(module) {}

    std::string name() const override { return "simplify-cfg"; }
    bool run() override;
};

}  // namespace IR
