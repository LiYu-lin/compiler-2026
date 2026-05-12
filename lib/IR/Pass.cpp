#include "Pass.h"

namespace IR {

void ModulePass::dumpModule(std::ostream& os) const {
    module.gen(os);
}

}  // namespace IR
