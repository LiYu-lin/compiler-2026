#pragma once

#include "ir/module.h"

#include <map>
#include <ostream>
#include <string>

namespace IR {

class Pass {
public:
    virtual ~Pass() = default;
    virtual std::string name() const = 0;
};

class ModulePass : public Pass {
public:
    explicit ModulePass(Module& module) : module(module) {}
    ~ModulePass() override = default;

    virtual bool run() = 0;
    virtual std::map<std::string, int> stats() const { return {}; }

    void dumpModule(std::ostream& os) const;

protected:
    Module& module;
};

}  // namespace IR
