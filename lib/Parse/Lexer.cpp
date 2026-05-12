/**
 * @file Lexer.cpp
 * @author Dexer Matters (dexermatters@gmail.com)
 * @brief Implementation of the Lexer class.
 * @version 0.1
 * @date 2025-05-17
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "../../include/Parse/Lexer.h"
#include <memory>

namespace frontend {

std::set<std::string> Token::reserved = {
    "if",       "else",  "while", "return", "break",
    "continue", "const", "int",   "float",  "void",
};

namespace token {

std::string Ident::toString() const { return "Ident(" + v + ")"; }

std::string FloatConst::toString() const {
    return "FloatConst(" + std::to_string(v) + ")";
}

std::string IntConst::toString() const {
    return "IntConst(" + std::to_string(v) + ")";
}

std::string Operator::toString() const { return "Operator(" + v + ")"; }

std::string Delimiter::toString() const { return "Delimiter(" + v + ")"; }

} // namespace token

std::string Lexer::Error::toString() const {
    return "Error: " + message + " at line " + std::to_string(line) +
           ", column " + std::to_string(column) + ", position " +
           std::to_string(tokPos);
}

std::string Lexer::readAll(std::stringstream &buffer) {
    std::string str;
    while (buffer >> str) {
        if (str == "EOF") {
            break;
        }
    }
    return str;
}

char Lexer::peek() { return input.peek(); }

char Lexer::get() {
    char c;
    if (input.get(c)) {
        tokPos++;
        column++;
        if (c == '\n') {
            line++;
            column = 1;
        }
        return c;
    }
    return EOF;
}

void Lexer::unget() {
    if (input) {
        input.unget();
        tokPos--;
        column--;
        if (column == 0) {
            line--;
            column = 1;
        }
    }
}

TokenPtr Lexer::nextIdentifier() {
    size_t start = tokPos;
    size_t startLine = line;
    size_t startColumn = column;
    std::string ident;
    char c;
    while (isalnum(c = get()) || c == '_') {
        ident += c;
    }
    //gzj: 回退一个字符，因为最后一个字符是不符合标识符�?
    unget();
    if (ident.empty()) {
        throw newError("Empty identifier");
    }
    return box<token::Ident>(ident, startLine, startColumn, start);
}

std::string Lexer::nextDecimalStr() {
    std::string num;
    char c;
    while (isdigit(c = get())) {
        num += c;
    }
    unget();
    if (num.empty()) {
        throw newError("Unexpected decimal number");
    }
    return num;
}

TokenPtr Lexer::nextHexadecimal() {
    size_t start = tokPos;
    size_t startLine = line;
    size_t startColumn = column;
    std::string num;
    char c;
    while (isxdigit(c = get())) {
        num += c;
    }
    unget();
    if (num.empty()) {
        throw newError("Unexpected hexadecimal number");
    }
    return box<token::IntConst>(std::stoi(num, nullptr, 16), startLine,
                                startColumn, start);
}

TokenPtr Lexer::nextOctal() {
    size_t start = tokPos;
    size_t startLine = line;
    size_t startColumn = column;
    std::string num;
    char c;
    while (isdigit(c = get()) && c < '8') {
        num += c;
    }
    unget();
    if (num.empty()) {
        return box<token::IntConst>(0, startLine, startColumn, start);
    }
    return box<token::IntConst>(std::stoi(num, nullptr, 8), startLine,
                                startColumn, start);
}

TokenPtr Lexer::nextNumber() {
    char c = get();
    auto frac = [&] {
        size_t start = tokPos;
        size_t startLine = line;
        size_t startColumn = column;
        auto decimal = "0." + nextDecimalStr();
        return box<token::FloatConst>(std::stof(decimal), startLine,
                                      startColumn, start);
    };
    if (c == '0') {
        if (peek() == 'x') {
            get();
            return nextHexadecimal();
        } else if (peek() == '.') {
            get();
            return frac();
        } else {
            return nextOctal();
        }
    } else if (c == '.') {
        return frac();
    } else {
        unget();
        size_t start = tokPos;
        size_t startLine = line;
        size_t startColumn = column;
        auto intPart = nextDecimalStr();
        if (peek() == '.') {
            get();
            auto decimal = intPart + "." + nextDecimalStr();
            return box<token::FloatConst>(std::stof(decimal), startLine,
                                          startColumn, start);
        } else {
            return box<token::IntConst>(std::stoi(intPart), startLine,
                                        startColumn, start);
        }
    }
}

void Lexer::skipLineComment() {
    char c;
    while ((c = get()) != '\n' && c != EOF) {
        // Skip
    }
}

void Lexer::skipMultilineComment() {
    char c;
    while ((c = get()) != EOF) {
        if (c == '*') {
            if (peek() == '/') {
                get();
                break;
            }
        }
    }
}

std::shared_ptr<Token> Lexer::nextToken() {
    char c;
    while ((c = get()) != EOF) {
        if (isspace(c) || c == '\r' || c == '\t' || c == '\n') {
            continue;
        } else if (c == '/') {
            if (peek() == '/') {
                get();
                skipLineComment();
                continue;
            } else if (peek() == '*') {
                get();
                skipMultilineComment();
                continue;
            }
        } else if (isalpha(c) || c == '_') {
            unget();
            return nextIdentifier();
        } else if (isdigit(c) || c == '.') {
            unget();
            return nextNumber();
        } else if (c == '.') {
            return box<token::Delimiter>(".", line, column, tokPos);
        } else if (c == ';') {
            return box<token::Delimiter>(";", line, column, tokPos);
        } else if (c == ',') {
            return box<token::Delimiter>(",", line, column, tokPos);
        } else if (c == '(') {
            return box<token::Delimiter>("(", line, column, tokPos);
        } else if (c == ')') {
            return box<token::Delimiter>(")", line, column, tokPos);
        } else if (c == '{') {
            return box<token::Delimiter>("{", line, column, tokPos);
        } else if (c == '}') {
            return box<token::Delimiter>("}", line, column, tokPos);
        } else if (c == '[') {
            return box<token::Delimiter>("[", line, column, tokPos);
        } else if (c == ']') {
            return box<token::Delimiter>("]", line, column, tokPos);
        } else if (c == '+') {
            return box<token::Operator>("+", line, column, tokPos);
        } else if (c == '-') {
            return box<token::Operator>("-", line, column, tokPos);
        } else if (c == '*') {
            return box<token::Operator>("*", line, column, tokPos);
        } else if (c == '/') {
            return box<token::Operator>("/", line, column, tokPos);
        } else if (c == '%') {
            return box<token::Operator>("%", line, column, tokPos);
        } else if (c == '&') {
            if (peek() == '&') {
                get();
                return box<token::Operator>("&&", line, column, tokPos);
            }
            return box<token::Operator>("&", line, column, tokPos);
        } else if (c == '|') {
            if (peek() == '|') {
                get();
                return box<token::Operator>("||", line, column, tokPos);
            }
            return box<token::Operator>("|", line, column, tokPos);
        } else if (c == '=') {
            if (peek() == '=') {
                get();
                return box<token::Operator>("==", line, column, tokPos);
            }
            return box<token::Operator>("=", line, column, tokPos);
        } else if (c == '!') {
            if (peek() == '=') {
                get();
                return box<token::Operator>("!=", line, column, tokPos);
            }
            return box<token::Operator>("!", line, column, tokPos);
        } else if (c == '<') {
            if (peek() == '=') {
                get();
                return box<token::Operator>("<=", line, column, tokPos);
            }
            return box<token::Operator>("<", line, column, tokPos);
        } else if (c == '>') {
            if (peek() == '=') {
                get();
                return box<token::Operator>(">=", line, column, tokPos);
            }
            return box<token::Operator>(">", line, column, tokPos);
        } else {
            throw newError("Unexpected character: " + std::string(1, c));
        }
    }
    return nullptr; // EOF
}

Lexer::Error Lexer::newError(const std::string &message) const {
    return Error(message, line, column, tokPos);
}

} // namespace frontend
