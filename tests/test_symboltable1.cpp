#include <iostream>
#include <memory>
#include "Parse/Grammar.h"
#include "SymbolTable/SymbolTable.h"
#include "SymbolTable/SymbolBuild.h"
using namespace frontend::parser;

std::string code = R"(
int main(int c) {
    int a = 5;
    if (a >= 10 && a < 20) {
        a = a + 1;
    } else {
        a = a - 1;
    };
    return 0;
}
)";




void testSymbolTable() {
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
    frontend::SymbolTable table;
    frontend::SymbolBuilder builder(table);
    builder.build(ast);
    table.dump();
    std::cout << "AST constructed and symbol table built successfully.\n";
  } catch (const frontend::ParserError &e) {
    std::cerr << "Error: " << e.toString() << std::endl;
  }
  catch (const std::string &s) {
    std::cerr << "Caught string exception: " << s << std::endl;
}


}


int main() {
    testSymbolTable();
    return 0;
}
