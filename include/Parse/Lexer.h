/**
 * @file Lexer.h
 * @author Dexer Matters (dexermatters@gmail.com)
 * @brief A file containing the definition of the Lexer class.
 * @version 0.1
 * @date 2025-05-17
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <cstddef>
#include <istream>
#include <memory>
#include <sstream>
#include <string>

#define box std::make_unique

namespace frontend {

class Token {
public:
    enum class Type {
        Identifier,
        Keyword,
        FloatConst,
        IntConst,
        Operator,
        Delimiter,
    };
    Type type;
    size_t line;
    size_t column;
    size_t tokPos;
    Token() = default;
    Token(Type type) : type(type) {}
    Token(Type type, size_t line, size_t column, size_t tokPos)
        : type(type), line(line), column(column), tokPos(tokPos) {}

    inline bool is(Type t) const { return type == t; }
    virtual std::string toString() const = 0;
    virtual ~Token() = default;
};

using TokenPtr = std::unique_ptr<Token>;

namespace token {
class Ident : public Token {
public:
    std::string v;
    Ident() : Token(Type::Identifier) {}
    Ident(std::string v) : v(v), Token(Type::Identifier) {}
    Ident(std::string v, size_t line, size_t column, size_t tokPos)
        : v(v), Token(Type::Identifier, line, column, tokPos) {}
    std::string toString() const override;
};

class FloatConst : public Token {
public:
    float v;
    FloatConst() : Token(Type::FloatConst) {}
    FloatConst(float v) : v(v), Token(Type::FloatConst) {}
    FloatConst(float v, size_t line, size_t column, size_t tokPos)
        : v(v), Token(Type::FloatConst, line, column, tokPos) {}
    std::string toString() const override;
};

class IntConst : public Token {
public:
    int v;
    IntConst() : Token(Type::IntConst) {}
    IntConst(int v) : v(v), Token(Type::IntConst) {}
    IntConst(int v, size_t line, size_t column, size_t tokPos)
        : v(v), Token(Type::IntConst, line, column, tokPos) {}
    std::string toString() const override;
};

class Operator : public Token {
public:
    std::string v;
    Operator() : Token(Type::Operator) {}
    Operator(std::string v) : v(v), Token(Type::Operator) {}
    Operator(std::string v, size_t line, size_t column, size_t tokPos)
        : v(v), Token(Type::Operator, line, column, tokPos) {}
    std::string toString() const override;
};

class Delimiter : public Token {
public:
    std::string v;
    Delimiter() : Token(Type::Delimiter) {}
    Delimiter(std::string v) : v(v), Token(Type::Delimiter) {}
    Delimiter(std::string v, size_t line, size_t column, size_t tokPos)
        : v(v), Token(Type::Delimiter, line, column, tokPos) {}
    std::string toString() const override;
};

} // namespace token

class Lexer {
public:
    struct Error {
        std::string message;
        size_t line;
        size_t column;
        size_t tokPos;
        Error() = default;
        Error(std::string message, size_t line, size_t column, size_t tokPos)
            : message(message), line(line), column(column), tokPos(tokPos) {}
        std::string toString() const;
    };

private:
    std::istream &input;
    size_t line = 1;
    size_t column = 1;
    size_t tokPos = 0;

    std::string readAll(std::stringstream &buffer);
    char peek();
    char get();
    void unget();
    TokenPtr nextIdentifier();
    std::string nextDecimalStr();
    TokenPtr nextHexadecimal();
    TokenPtr nextOctal();
    TokenPtr nextNumber();
    void skipLineComment();
    void skipMultilineComment();

public:
    Lexer() = delete;
    Lexer(std::istream &input) : input(input) {}
    std::unique_ptr<Token> nextToken();
    Error newError(const std::string &message) const;
};

} // namespace frontend