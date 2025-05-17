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
    try {
        while (auto token = lexer.nextToken()) {
            std::cout << token->toString() << std::endl;
        }

    } catch (const frontend::Lexer::Error &e) {
        std::cerr << e.toString() << std::endl;
    }
    return 0;
}