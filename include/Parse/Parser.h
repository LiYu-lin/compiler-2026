/**
 * @file Parser.h
 * @author Dexer Matters (dexermatters@gmail.com)
 * @brief Header file for the Parser class.
 * @version 0.1
 * @date 2025-05-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include "Parse/AST.h"
#include "Parse/Lexer.h"
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

namespace frontend {
class Parser {
private:
    std::vector<TokenPtr> tokens;
    std::vector<TokenPtr>::iterator current;

    using ParserRule = std::function<ast::ASTNodePtr()>;
    using ParserError = std::string;

    TokenPtr next() {
        if (current == tokens.end()) {
            throw nullptr;
        }
        return *current++;
    }

    bool ok(std::function<TokenPtr()> rule) {
        try {
            rule();
            return true;
        } catch (const ParserError &e) {
            return false;
        }
    }

    template <typename T> std::shared_ptr<T> next_tok(Token::Type type) {
        if (current == tokens.end()) {
            throw nullptr;
        }
        auto token = *current++;
        if (!token->is(type)) {
            throw unexpectedTokenError(token);
        }
        return std::shared_ptr<T>(static_cast<T *>(token.get()));
    }

    std::shared_ptr<token::Ident> next_ident() {
        auto tok = next_tok<token::Ident>(Token::Type::Identifier);
        if (Token::reserved.count(tok->v)) {
            throw unexpectedTokenError(tok, "identifier");
        }
        return tok;
    }

    TokenPtr next_expect(std::function<bool(TokenPtr)> predicate) {
        if (current == tokens.end()) {
            throw nullptr;
        }
        auto token = *current++;
        if (!predicate(token)) {
            throw unexpectedTokenError(token);
        }
        return token;
    }

    TokenPtr next_expect_ident(const std::string &ident) {
        return next_expect([&](TokenPtr tok) {
            return tok->is(Token::Type::Identifier) &&
                   !Token::reserved.count(
                       static_cast<token::Ident *>(tok.get())->v) &&
                   static_cast<token::Ident *>(tok.get())->v == ident;
        });
    }

    TokenPtr next_expect_delimiter(const std::string &delim) {
        return next_expect([&](TokenPtr tok) {
            return tok->is(Token::Type::Delimiter) &&
                   static_cast<token::Delimiter *>(tok.get())->v == delim;
        });
    }

    TokenPtr next_expect_op(const std::string &op) {
        return next_expect([&](TokenPtr tok) {
            return tok->is(Token::Type::Operator) &&
                   static_cast<token::Operator *>(tok.get())->v == op;
        });
    }

    TokenPtr next_expect_str(const std::string &str) {
        return next_expect([&](TokenPtr tok) {
            return tok->is(Token::Type::Identifier) &&
                   static_cast<token::Ident *>(tok.get())->v == str;
        });
    }

    TokenPtr peek_expect(std::function<bool(TokenPtr)> predicate) {
        if (current == tokens.end()) {
            throw nullptr;
        }
        auto token = *current;
        if (!predicate(token)) {
            throw unexpectedTokenError(token);
        }
        return token;
    }

    TokenPtr peek() {
        if (current == tokens.end()) {
            throw nullptr;
        }
        return *current;
    }

    TokenPtr previous() {
        if (current == tokens.begin()) {
            return nullptr;
        }
        return *(--current);
    }

    ast::ASTNodePtr try_or(ParserRule rule, ParserRule fallback) {
        auto saved = current;
        try {
            return rule();
        } catch (const ParserError &e) {
            current = saved;
            return fallback();
        }
    }

    ast::ASTNodePtr
    try_branches(const std::initializer_list<ParserRule> &rules) {
        auto saved = current;
        size_t i = 0;
        for (const auto &rule : rules) {
            if (i == rules.size() - 1) {
                return rule();
            }
            try {
                return rule();
            } catch (const ParserError &e) {
                current = saved;
            }
            i++;
        }
    }

public:
    Parser() = default;
    Parser(std::string code) {
        std::istringstream input(code);
        auto lexer = Lexer(input);
        tokens.reserve(256);
        while (auto token = lexer.nextToken()) {
            tokens.push_back(std::move(token));
        }
        current = tokens.begin();
    }
    ~Parser() = default;

    template <typename T> static std::string unexpectedTokenError(T &token) {
        return "Unexpected token: " + token->toString();
    }

    template <typename T>
    static std::string unexpectedTokenError(T &token,
                                            const std::string &expected) {
        return "Unexpected token: " + token->toString() +
               ", expected: " + expected;
    }

    static std::string unexpectedEOFError() { return "Unexpected end of file"; }

private:
    // Rule Definitions

    ast::ASTNodePtr parseCompUnit();
    ast::ASTNodePtr parseDecl();
    ast::ASTNodePtr parseConstDecl();
    ast::ASTNodePtr parseConstDef();
    ast::ASTNodePtr parseConstInitVal();
    ast::ASTNodePtr parseVarDecl();
    ast::ASTNodePtr parseVarDef();
    ast::ASTNodePtr parseInitVal();
    ast::ASTNodePtr parseFuncDef();
    ast::ASTNodePtr parseFuncFParams();
    ast::ASTNodePtr parseFuncFParam();
    ast::ASTNodePtr parseBlock();
    ast::ASTNodePtr parseBlockItem();
    ast::ASTNodePtr parseStmt();
    ast::ASTNodePtr parseAssignStmt();
    ast::ASTNodePtr parseExpStmt();
    ast::ASTNodePtr parseBlockStmt();
    ast::ASTNodePtr parseIfStmt();
    ast::ASTNodePtr parseWhileStmt();
    ast::ASTNodePtr parseBreakStmt();
    ast::ASTNodePtr parseContinueStmt();
    ast::ASTNodePtr parseReturnStmt();
    ast::ASTNodePtr parseExp();
    ast::ASTNodePtr parseCond();
    ast::ASTNodePtr parseLVal();
    ast::ASTNodePtr parsePrimaryExp();
    ast::ASTNodePtr parseUnaryExp();
    ast::ASTNodePtr parseUnaryExpOp();
    ast::ASTNodePtr parseUnaryExpCall();
    ast::ASTNodePtr parseMulExp();
    ast::ASTNodePtr parseMulExpOp();
    ast::ASTNodePtr parseAddExp();
    ast::ASTNodePtr parseAddExpOp();
    ast::ASTNodePtr parseRelExp();
    ast::ASTNodePtr parseRelExpOp();
    ast::ASTNodePtr parseEqExp();
    ast::ASTNodePtr parseEqExpOp();
    ast::ASTNodePtr parseLAndExp();
    ast::ASTNodePtr parseLAndExpOp();
    ast::ASTNodePtr parseLOrExp();
    ast::ASTNodePtr parseLOrExpOp();
    ast::ASTNodePtr parseConstExp();
};

} // namespace frontend