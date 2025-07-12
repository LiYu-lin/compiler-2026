#include "Parse/Grammar.h"
#include "Parse/Lexer.h"
#include "Parse/Parser.h"
#include "ir/module.h"
#include "GenIR/Visitor.h"
#include <iostream>
#include <sstream>

using namespace frontend::parser;

std::string code = R"(
const int b=10;
int main() {
    int a = 2;
    if(a>1){
        int c = 3;
    }
    return 0;
}
)";

int main() {
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
    
    try {
        auto ast = parser.run(
            frontend::TokenPtrIterator(tokens.begin(), tokens.end()));
        std::cout << "Parsed successfully: " << ast->toString(0) << std::endl;

        std::cout << "Generating IR..." << std::endl;
        IR::Module mod("test_module");

        frontend::visitor::Visitor visitor(mod);
        std::cout << "Visitor created." << std::endl;
        
        try {
            visitor.visit(*ast);
            std::cout << "IR generation completed." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "IR generation failed: " << e.what() << std::endl;
            return 1;
        } catch (...) {
            std::cerr << "Unknown error during IR generation" << std::endl;
            return 1;
        }

        if (mod.getFunctionList().empty()) {
            std::cerr << "Error: No functions generated in IR" << std::endl;
            return 1;
        }
        
        // 输出IR到终端
        mod.gen(std::cout);
        
        // 简单验证main函数是否存在
        bool hasMain = false;
        for (auto func : mod.getFunctionList()) {
            if (func->getIRName() == "main") {
                hasMain = true;
                break;
            }
        }
        
        if (!hasMain) {
            std::cerr << "Error: main function not found in generated IR" << std::endl;
            return 1;
        }
        
        std::cout << "IR generation test passed!" << std::endl;
        
    } catch (const frontend::ParserError &e) {
        std::cerr << "Error: " << e.toString() << std::endl;
        return 1;
    }
    
    return 0;
}