#include "PassManager.h"

#include <iostream>

namespace IR {

PassManager::PassManager(Module& module, PassManagerOptions options)
    : module(module), options(std::move(options)) {}

bool PassManager::run() {
    bool changed = false;
    for (const auto& pass : passes) {
        dumpBefore(*pass);
        changed |= pass->run();
        dumpAfter(*pass);
        verifyAfter(*pass);
    }
    return changed;
}

void PassManager::dumpBefore(const ModulePass& pass) const {
    if (options.printBefore != pass.name()) {
        return;
    }

    std::cerr << "===== Before " << pass.name() << " =====\n\n";
    pass.dumpModule(std::cerr);
    std::cerr << "\n\n";
}

void PassManager::dumpAfter(const ModulePass& pass) const {
    if (!options.verbose && options.printAfter != pass.name()) {
        return;
    }

    std::cerr << "===== After " << pass.name() << " =====\n\n";
    pass.dumpModule(std::cerr);
    std::cerr << "\n\n";

    auto passStats = pass.stats();
    if (!passStats.empty()) {
        std::cerr << "[stats] " << pass.name() << "\n";
        for (const auto& [key, value] : passStats) {
            std::cerr << "  " << key << " : " << value << "\n";
        }
    }
}

void PassManager::verifyAfter(const ModulePass& pass) const {
    if (!options.verifyEachPass) {
        return;
    }

    // Placeholder hook for a future IR verifier.
    std::cerr << "[verify] " << pass.name() << " (not implemented yet)\n";
}

}  // namespace IR
