#include "Parse/Grammar.h"
#include "Parse/Lexer.h"
#include "Parse/Parser.h"
#include <iostream>

using namespace frontend::parser;

std::string code = R"(
const int MAX = 100;
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
    } catch (const frontend::ParserError &e) {
        std::cerr << "Error: " << e.toString() << std::endl;
    }
}