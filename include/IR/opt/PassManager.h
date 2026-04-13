#pragma once

#include "ir/opt/Pass.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace IR {

struct PassManagerOptions {
    bool verbose = false;
    bool verifyEachPass = false;
    std::string printBefore;
    std::string printAfter;
};

class PassManager {
public:
    explicit PassManager(Module& module, PassManagerOptions options = {});
    ~PassManager() = default;

    template <typename T, typename... Args>
    void addPass(Args&&... args) {
        static_assert(std::is_base_of_v<ModulePass, T>,
                      "PassManager only supports ModulePass-derived passes");
        passes.push_back(std::make_unique<T>(module, std::forward<Args>(args)...));
    }

    bool run();

    Module& getModule() { return module; }
    const Module& getModule() const { return module; }

private:
    Module& module;
    PassManagerOptions options;
    std::vector<std::unique_ptr<ModulePass>> passes;

    void dumpBefore(const ModulePass& pass) const;
    void dumpAfter(const ModulePass& pass) const;
    void verifyAfter(const ModulePass& pass) const;
};

}  // namespace IR
