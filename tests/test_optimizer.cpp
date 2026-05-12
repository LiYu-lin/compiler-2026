#include "GenIR/Visitor.h"
#include "Parse/Grammar.h"
#include "Parse/Lexer.h"
#include "Parse/Parser.h"
#include "ir/module.h"
#include "ir/opt/Passes.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

IR::Module* generateOptimizedIR(const std::string& code) {
    auto parser = frontend::parser::comp_unit();
    auto stream = std::stringstream(code);
    auto lexer = frontend::Lexer(stream);

    std::vector<frontend::TokenPtr> tokens;
    while (auto token = lexer.nextToken()) {
        tokens.push_back(token);
    }

    auto ast = parser.run(frontend::TokenPtrIterator(tokens.begin(), tokens.end()));
    auto* module = new IR::Module("optimizer_test_module");
    frontend::visitor::Visitor visitor(*module);
    visitor.visit(*ast);

    IR::PassManager passManager(*module);
    passManager.addPass<IR::SimplifyCFG>();
    passManager.addPass<IR::InstCombine>();
    passManager.addPass<IR::MemoryOpt>();
    passManager.addPass<IR::DCE>();
    passManager.addPass<IR::SimplifyCFG>();
    passManager.addPass<IR::MemoryOpt>();
    passManager.addPass<IR::DCE>();
    passManager.run();
    return module;
}

void requireMissing(const std::string& text, const std::string& needle) {
    if (text.find(needle) != std::string::npos) {
        throw std::runtime_error("Optimized IR still contains: " + needle);
    }
}

void requireContains(const std::string& text, const std::string& needle) {
    if (text.find(needle) == std::string::npos) {
        throw std::runtime_error("Optimized IR is missing: " + needle);
    }
}

}

int main() {
    try {
        const std::string code = R"(
int main() {
    int a = 4;
    int b = a + 0;
    int c = b * 1;
    if (1 == 1) {
        c = c + 0;
    } else {
        c = c * 0;
    }
    c + 123;
    return c;
}
)";

        std::unique_ptr<IR::Module> module(generateOptimizedIR(code));
        std::ostringstream ir;
        module->gen(ir);
        const auto output = ir.str();

        requireContains(output, "define function @main");
        requireContains(output, "return i32");
        requireMissing(output, "then label");
        requireMissing(output, "add int");
        requireMissing(output, "mul int");
        requireMissing(output, " 123");
        requireMissing(output, "store i32 @");

        std::cout << "Optimizer test passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Optimizer test failed: " << e.what() << std::endl;
        return 1;
    }
}
