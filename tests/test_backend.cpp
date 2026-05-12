#ifdef COMPILER_BUILD_TESTS
#include "module.h"
#include "component.h"
#include "Grammar.h"
#include "Lexer.h"
#include "Parser.h"
#include "Visitor.h"
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>

using namespace backend;
using namespace frontend::parser;

std::string testCode = R"(
int main() {
    int a = 5;
    if (a >= 10 && a < 20) {
        a = a + 1;
    } else {
        a = a - 1;
    };
    return 0;
}
)";

std::string callTestCode = R"(
int add(int x, int y) {
    return x + y;
}
int main() {
    return add(3, 4);
}
)";

IR::Module* generateIRFromSource(const std::string& code) {
    auto parser = comp_unit();
    auto istream = std::stringstream(code);
    auto lexer = frontend::Lexer(istream);
    auto tokens = [&lexer] {
        std::vector<frontend::TokenPtr> tokens;
        frontend::TokenPtr token;
        while ((token = lexer.nextToken()) != nullptr) {
            tokens.push_back(token);
        };
        return tokens;
    }();

    auto ast = parser.run(frontend::TokenPtrIterator(tokens.begin(), tokens.end()));
    IR::Module* mod = new IR::Module("test_module");
    frontend::visitor::Visitor visitor(*mod);
    visitor.visit(*ast);

    return mod;
}

bool testFullBackendPipeline() {
    std::cout << "\n=== Testing Full Backend Pipeline ===" << std::endl;

    try {
        std::cout << "Generating IR from source code..." << std::endl;
        IR::Module* irMod = generateIRFromSource(testCode);

        irMod->gen(std::cout);

        std::cout << "Converting to backend assembly..." << std::endl;
        AsmModule asmMod(irMod);
        auto asmOutput = asmMod.output();

        std::cout << "\nGenerated RISC-V Assembly:\n";
        std::cout << asmOutput << std::endl;

        auto requireContains = [&](const std::string& needle) {
            if (asmOutput.find(needle) == std::string::npos) {
                throw std::runtime_error("Missing expected assembly fragment: " + needle);
            }
        };

        requireContains("addi sp, sp, -16");
        requireContains("addi sp, sp, 16");
        requireContains(", zero, Label_5");
        requireContains("mv a0, zero");
        requireContains("ret");
        if (asmOutput.find("@main") != std::string::npos || asmOutput.find("@memset") != std::string::npos) {
            throw std::runtime_error("Assembly should not expose IR-style symbol names");
        }
        if (asmOutput.find("memset:") != std::string::npos) {
            throw std::runtime_error("External functions should not be emitted as empty definitions");
        }
        if (std::regex_search(asmOutput, std::regex(R"(\b(gp|tp|s0)\b)"))) {
            throw std::runtime_error("Allocator used reserved or callee-saved registers in this simple test");
        }
        if (!std::regex_search(asmOutput, std::regex(R"(li\s+\w+,\s+5)"))) {
            throw std::runtime_error("Missing immediate load for constant 5");
        }
        if (!std::regex_search(asmOutput, std::regex(R"(sw\s+\w+,\s+0\(sp\))"))) {
            throw std::runtime_error("Missing store to stack slot");
        }
        if (!std::regex_search(asmOutput, std::regex(R"(lw\s+\w+,\s+0\(sp\))"))) {
            throw std::runtime_error("Missing load from stack slot");
        }
        if (!std::regex_search(asmOutput, std::regex(R"(bne\s+\w+,\s+zero,\s+Label_5)"))) {
            throw std::runtime_error("Missing conditional branch to Label_5");
        }
        if (std::regex_search(asmOutput, std::regex(R"(\b(sw|lw)\s+(\w+),\s+0\(\2\))"))) {
            throw std::runtime_error("Backend reused the same register as both data and stack address");
        }
        if (std::regex_search(asmOutput, std::regex(R"(\bslt\s+\w+,\s+(\w+),\s+\1)"))) {
            throw std::runtime_error("Backend collapsed both compare operands into one register");
        }
        if (asmOutput.find("unknown ") != std::string::npos) {
            throw std::runtime_error("Backend emitted unknown pseudo instruction");
        }

        std::cout << "Backend pipeline test passed!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in backend pipeline: " << e.what() << std::endl;
        return false;
    }
}

bool testFunctionCallLowering() {
    std::cout << "\n=== Testing Function Call Lowering ===" << std::endl;

    try {
        IR::Module* irMod = generateIRFromSource(callTestCode);
        AsmModule asmMod(irMod);
        auto asmOutput = asmMod.output();

        if (asmOutput.find("\tcall add\n") == std::string::npos) {
            throw std::runtime_error("Missing lowered function call");
        }
        if (asmOutput.find("sw ra, 0(sp)") == std::string::npos ||
            asmOutput.find("lw ra, 0(sp)") == std::string::npos) {
            throw std::runtime_error("Caller/callee return address handling was not emitted");
        }
        if (asmOutput.find("sw a1, 0(a1)") != std::string::npos) {
            throw std::runtime_error("Parameter lowering clobbered an argument register");
        }
        if (asmOutput.find("unknown ") != std::string::npos) {
            throw std::runtime_error("Backend emitted unknown instruction in call lowering");
        }

        std::cout << "Function call lowering test passed!" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in function call lowering: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    bool ok = true;
    ok = testFullBackendPipeline() && ok;
    ok = testFunctionCallLowering() && ok;
    return ok ? 0 : 1;
}
#endif
