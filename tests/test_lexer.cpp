#include "Parse/Lexer.h"
#include <iostream>
#include <sstream>

std::string code = R"(
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
    std::istringstream input(code);
    frontend::Lexer lexer(input);
    while (auto token = lexer.nextToken()) {
        try {
            std::cout << token->toString() << std::endl;
        } catch (frontend::Lexer::Error &e) {
            std::cerr << e.toString() << std::endl;
        }
    }
}