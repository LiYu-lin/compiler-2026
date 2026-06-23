#include "PassManager.h"
#include <iostream>

namespace IR {

PassManager::PassManager(Module& module, PassManagerOptions options)
    : module(module), options(std::move(options)) {}

bool PassManager::run() {
    bool overall_changed = false;
    int manager_safety_counter = 0;
    
    while (true) {
        if (++manager_safety_counter > 10) {
            break;
        }
        
        bool current_round_changed = false;
        for (const auto& pass : passes) {
            dumpBefore(*pass);
            bool pass_changed = pass->run();
            current_round_changed |= pass_changed;
            overall_changed |= pass_changed;
            dumpAfter(*pass);
            verifyAfter(*pass);
        }
        
        if (!current_round_changed) {
            break;
        }
    }
    
    return overall_changed;
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

    std::cerr << "[verify] " << pass.name() << " (not implemented yet)\n";
}

}