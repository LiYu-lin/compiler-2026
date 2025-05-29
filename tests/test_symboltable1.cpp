#include <iostream>
#include <memory>
#include "Parse/Grammar.h"
#include "SymbolTable/SymbolTable.h"
#include "SymbolTable/SymbolBuild.h"
using namespace frontend::parser;

std::string code = R"(
const int b=10;
int main() {
    int a = 2;
    if(a>1){
        int c = 3;
    }
    else {
        int d = 4;
        while (d < 10) {
            d = d + 1;
            if(d==9){
                int e = 5;
            }
        }

    }
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
