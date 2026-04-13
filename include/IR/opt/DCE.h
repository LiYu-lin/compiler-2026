#pragma once

#include "ir/opt/Pass.h"

namespace IR {

class DCE : public ModulePass {
public:
    explicit DCE(Module& module) : ModulePass(module) {}

    std::string name() const override { return "dce"; }
    bool run() override;
};

}  // namespace IR
