#include "../include/GenIR/Visitor.h"
#include "../include/Parse/Grammar.h"
#include "../include/Parse/Lexer.h"
#include "../include/Parse/Parser.h"
#include "../include/backend/component.h"
#include "../include/IR/module.h"
#include "../include/IR/opt/Passes.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) {
        throw std::runtime_error("Failed to open input file: " + path);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

frontend::TokenPtrs lexSource(const std::string& code) {
    auto stream = std::stringstream(code);
    auto lexer = frontend::Lexer(stream);

    frontend::TokenPtrs tokens;
    while (auto token = lexer.nextToken()) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string dumpTokensToString(const frontend::TokenPtrs& tokens) {
    std::ostringstream out;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& token = tokens[i];
        out << i << "\t"
            << token->line << ":" << token->column << "\t"
            << token->toString() << "\n";
    }
    return out.str();
}

frontend::ast::ASTNodePtr parseTokens(frontend::TokenPtrs& tokens) {
    auto parser = frontend::parser::comp_unit();
    return parser.run(frontend::TokenPtrIterator(tokens.begin(), tokens.end()));
}

IR::Module* buildModuleFromAST(const frontend::ast::ASTNodePtr& ast) {
    auto* module = new IR::Module("sysy_module");
    frontend::visitor::Visitor visitor(*module);
    visitor.visit(*ast);
    return module;
}

void writeOutput(const std::string& outputPath, const std::string& content) {
    if (outputPath.empty()) {
        std::cout << content;
        return;
    }

    std::ofstream output(outputPath, std::ios::out | std::ios::binary);
    if (!output) {
        throw std::runtime_error("Failed to open output file: " + outputPath);
    }
    output << content;
}

void printUsage(const char* argv0) {
    std::cerr
        << "Usage: " << argv0
        << " <input.sysy|input.sy> -S -o <output-file> [-O1]\n"
        << "       " << argv0
        << " [--dump-tokens|--dump-ast|--emit-raw-ir|--emit-ir|--emit-asm] <input.sysy|input.sy> [-o <output-file>]\n";
}

}

int main(int argc, char** argv) {
    try {
        bool emitIR = false;
        bool emitAsm = true;
        bool emitRawIR = false;
        bool dumpTokens = false;
        bool dumpAST = false;
        std::string inputPath;
        std::string outputPath;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--emit-ir") {
                emitIR = true;
                emitAsm = false;
                emitRawIR = false;
                dumpTokens = false;
                dumpAST = false;
            } else if (arg == "--emit-asm" || arg == "-S") {
                emitAsm = true;
                emitIR = false;
                emitRawIR = false;
                dumpTokens = false;
                dumpAST = false;
            } else if (arg == "--emit-raw-ir") {
                emitRawIR = true;
                emitAsm = false;
                emitIR = false;
                dumpTokens = false;
                dumpAST = false;
            } else if (arg == "--dump-tokens") {
                dumpTokens = true;
                emitAsm = false;
                emitIR = false;
                emitRawIR = false;
                dumpAST = false;
            } else if (arg == "--dump-ast") {
                dumpAST = true;
                emitAsm = false;
                emitIR = false;
                emitRawIR = false;
                dumpTokens = false;
            } else if (arg == "-O0" || arg == "-O1" || arg == "-O2") {
                // Competition compatibility: optimization levels are accepted
                // here and mapped onto the currently available pass pipeline.
                continue;
            } else if (arg == "-o" || arg == "--output") {
                if (i + 1 >= argc) {
                    throw std::runtime_error("Missing path after " + arg);
                }
                outputPath = argv[++i];
            } else if (arg == "-h" || arg == "--help") {
                printUsage(argv[0]);
                return 0;
            } else if (!arg.empty() && arg.front() == '-') {
                throw std::runtime_error("Unknown option: " + arg);
            } else if (inputPath.empty()) {
                inputPath = arg;
            } else {
                throw std::runtime_error("Unexpected extra positional argument: " + arg);
            }
        }

        if (inputPath.empty()) {
            printUsage(argv[0]);
            return 1;
        }

        auto source = readFile(inputPath);
        auto tokens = lexSource(source);

        if (dumpTokens) {
            writeOutput(outputPath, dumpTokensToString(tokens));
            return 0;
        }

        auto ast = parseTokens(tokens);

        if (dumpAST) {
            writeOutput(outputPath, ast->toString(0) + "\n");
            return 0;
        }

        std::unique_ptr<IR::Module> module(buildModuleFromAST(ast));
        if (emitRawIR) {
            std::ostringstream ir;
            module->gen(ir);
            writeOutput(outputPath, ir.str());
            return 0;
        }
        IR::PassManager passManager(*module);
        passManager.addPass<IR::SimplifyCFG>();
        passManager.addPass<IR::InstCombine>();
        passManager.addPass<IR::MemoryOpt>();
        passManager.addPass<IR::DCE>();
        passManager.addPass<IR::SimplifyCFG>();
        passManager.addPass<IR::MemoryOpt>();
        passManager.addPass<IR::DCE>();
        passManager.run();

        if (emitIR) {
            std::ostringstream ir;
            module->gen(ir);
            writeOutput(outputPath, ir.str());
            return 0;
        }

        if (emitAsm) {
            backend::AsmModule asmModule(module.get());
            writeOutput(outputPath, asmModule.output());
            return 0;
        }

        throw std::runtime_error("No emission mode selected");
    } catch (const std::exception& e) {
        std::cerr << "compiler error: " << e.what() << '\n';
        return 1;
    }
}
