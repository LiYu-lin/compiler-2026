#include "Parse/Grammar.h"
#include "Parse/Lexer.h"
#include "Parse/Parser.h"
#include <iostream>

using namespace frontend::parser;
// The parser stops when the function parameter  is an array.
std::string code = R"(
// 示例：使用 while 循环查找数组中的最大值
const int SIZE = 5;
 
// 函数：查找数组中的最大值
int findMax(int arr[SIZE]) {
    int max = arr[0];
    int i = 1;
    while (i < SIZE) {
        if (arr[i] > max) {
            max = arr[i];
        }
        i = i + 1;
    }
    return max;
}
 
int main() {
    int array[SIZE] = {1, 2, 3, 4, 5};
    int maxValue = findMax(array);
    return maxValue; // 返回数组中的最大值
};
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
    auto ast =
        parser.run(frontend::TokenPtrIterator(tokens.begin(), tokens.end()));
    std::cout << "Parsed successfully: " << ast->toString(0) << std::endl;
  } catch (const frontend::ParserError &e) {
    std::cerr << "Error: " << e.toString() << std::endl;
  }
}