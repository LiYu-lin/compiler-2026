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
        positionHistory.emplace_back(line, column);
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
    if (input && !positionHistory.empty()) {
        input.unget();
        tokPos--;
        auto [prevLine, prevColumn] = positionHistory.back();
        positionHistory.pop_back();
        line = prevLine;
        column = prevColumn;
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
    size_t start = tokPos;
    size_t startLine = line;
    size_t startColumn = column;
    std::string literal;
    bool isFloat = false;

    auto takeDigits = [&](auto predicate) {
        bool any = false;
        while (predicate(peek())) {
            literal += get();
            any = true;
        }
        return any;
    };

    auto takeDecimalExponent = [&]() {
        if (peek() != 'e' && peek() != 'E') {
            return false;
        }
        isFloat = true;
        literal += get();
        if (peek() == '+' || peek() == '-') {
            literal += get();
        }
        if (!isdigit(peek())) {
            throw newError("Unexpected decimal exponent");
        }
        takeDigits([](char ch) { return isdigit(ch); });
        return true;
    };

    auto takeHexExponent = [&]() {
        if (peek() != 'p' && peek() != 'P') {
            return false;
        }
        isFloat = true;
        literal += get();
        if (peek() == '+' || peek() == '-') {
            literal += get();
        }
        if (!isdigit(peek())) {
            throw newError("Unexpected hexadecimal exponent");
        }
        takeDigits([](char ch) { return isdigit(ch); });
        return true;
    };

    literal += get();

    if (literal == "0" && (peek() == 'x' || peek() == 'X')) {
        literal += get();
        bool hasDigits = takeDigits([](char ch) { return isxdigit(ch); });
        if (peek() == '.') {
            isFloat = true;
            literal += get();
            hasDigits = takeDigits([](char ch) { return isxdigit(ch); }) || hasDigits;
        }
        if (!hasDigits) {
            throw newError("Unexpected hexadecimal number");
        }
        takeHexExponent();
        if (isFloat) {
            return box<token::FloatConst>(std::stof(literal), startLine, startColumn, start);
        }
        return box<token::IntConst>(std::stoi(literal, nullptr, 16), startLine, startColumn, start);
    }

    if (literal == ".") {
        isFloat = true;
        if (!takeDigits([](char ch) { return isdigit(ch); })) {
            throw newError("Unexpected decimal number");
        }
        takeDecimalExponent();
        return box<token::FloatConst>(std::stof(literal), startLine, startColumn, start);
    }

    if (literal == "0") {
        if (peek() == '.') {
            isFloat = true;
            literal += get();
            takeDigits([](char ch) { return isdigit(ch); });
            takeDecimalExponent();
            return box<token::FloatConst>(std::stof(literal), startLine, startColumn, start);
        }
        if (peek() == 'e' || peek() == 'E') {
            takeDecimalExponent();
            return box<token::FloatConst>(std::stof(literal), startLine, startColumn, start);
        }
        std::string octalDigits;
        while (isdigit(peek())) {
            char ch = get();
            if (ch >= '8') {
                throw newError("Unexpected octal number");
            }
            octalDigits += ch;
        }
        if (octalDigits.empty()) {
            return box<token::IntConst>(0, startLine, startColumn, start);
        }
        return box<token::IntConst>(std::stoi(octalDigits, nullptr, 8), startLine, startColumn, start);
    }

    takeDigits([](char ch) { return isdigit(ch); });
    if (peek() == '.') {
        isFloat = true;
        literal += get();
        takeDigits([](char ch) { return isdigit(ch); });
    }
    takeDecimalExponent();

    if (isFloat) {
        return box<token::FloatConst>(std::stof(literal), startLine, startColumn, start);
    }
    return box<token::IntConst>(std::stoi(literal), startLine, startColumn, start);
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
            return box<token::Operator>("/", line, column, tokPos);
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



