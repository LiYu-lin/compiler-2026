#include "Parse/Grammar.h"
#include "Parse/Lexer.h"
#include "Parse/Parser.h"
#include "ir/module.h"
#include "ir/IRBuilder.h"
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

        // === IR生成部分 ===
        IR::Module mod("test_module");
        IR::IRBuilder builder;
        mod.gen(std::cout);      // 输出IR到终端

    } catch (const frontend::ParserError &e) {
        std::cerr << "Error: " << e.toString() << std::endl;
    }
}