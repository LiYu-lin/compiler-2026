/**
 * @file Parser.cpp
 * @author Dexer Matters (dexermatters@gmail.com)
 * @brief Parser class implementation
 * @version 0.1
 * @date 2025-05-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "Parse/Parser.h"
#include "Parse/Lexer.h"

#define self(func, ...) [this]() { return this->func(__VA_ARGS__); }
#define lam(exp) [this]() { return exp; }
#define check(exp) ok([this]() { return exp; })

namespace frontend {

// Rule Definitions

ast::ASTNodePtr Parser::parseCompUnit() {
    auto decls = ast::ASTNodePtrs();
    ast::ASTNodePtr ast;
    while ((ast = try_or(self(parseFuncDef), self(parseDecl))) != nullptr) {
        decls.push_back(ast);
    }
    if (current != tokens.end()) {
        throw unexpectedEOFError();
    };
    return uni<ast::CompUnit>(std::move(decls));
}

ast::ASTNodePtr Parser::parseDecl() {
    return try_or(self(parseConstDecl), self(parseVarDecl));
}

ast::ASTNodePtr Parser::parseConstDecl() {
    next_expect_str("const");

    auto btype = next_tok<token::Ident>(Token::Type::Identifier);
    ast::ASTNode::BType btypeEnum;
    if (btype->v == "int") {
        btypeEnum = ast::ASTNode::BType::Int;
    } else if (btype->v == "float") {
        btypeEnum = ast::ASTNode::BType::Float;
    } else {
        throw unexpectedTokenError(btype, "int or float");
    }

    auto constDefs = ast::ASTNodePtrs();
    constDefs.push_back(parseConstDef());

    while (current != tokens.end()) {
        if (check(next_expect_delimiter(";"))) {
            break;
        }
        if (check(next_expect_delimiter(","))) {
            constDefs.push_back(parseConstDef());
        } else {
            throw unexpectedTokenError(current, ", or ;");
        }
    }

    return uni<ast::ConstDecl>(btypeEnum, std::move(constDefs));
}

ast::ASTNodePtr Parser::parseConstDef() {
    auto ident = next_ident();
    auto dimension = ast::ASTNodePtrs();
    while (current != tokens.end()) {
        if (check(next_expect_op("="))) {
            break;
        }
        next_expect_delimiter("[");
        dimension.push_back(parseConstExp());
        next_expect_delimiter("]");
    }
    auto initVal = parseConstInitVal();
    return uni<ast::ConstDef>(ident->v, std::move(dimension),
                              std::move(initVal));
};

ast::ASTNodePtr Parser::parseConstInitVal() {
    if (check(next_expect_delimiter("{"))) {
        auto initVals = ast::ASTNodePtrs();
        while (current != tokens.end()) {
            if (check(next_expect_delimiter("}"))) {
                break;
            }
            initVals.push_back(parseConstInitVal());
            if (check(next_expect_delimiter(","))) {
                continue;
            }
        }
        return uni<ast::ConstInitVal>(std::move(initVals));
    } else {
        previous();
        return uni<ast::ConstInitVal>(parseConstExp());
    }
}

ast::ASTNodePtr Parser::parseVarDecl() {
    auto btype = next_tok<token::Ident>(Token::Type::Identifier);
    ast::ASTNode::BType btypeEnum;
    if (btype->v == "int") {
        btypeEnum = ast::ASTNode::BType::Int;
    } else if (btype->v == "float") {
        btypeEnum = ast::ASTNode::BType::Float;
    } else {
        throw unexpectedTokenError(btype, "int or float");
    }

    auto varDefs = ast::ASTNodePtrs();
    varDefs.push_back(parseVarDef());

    while (current != tokens.end()) {
        if (check(next_expect_delimiter(";"))) {
            break;
        }
        if (check(next_expect_delimiter(","))) {
            varDefs.push_back(parseVarDef());
        } else {
            throw unexpectedTokenError(current, ", or ;");
        }
    }

    return uni<ast::VarDecl>(btypeEnum, std::move(varDefs));
}

ast::ASTNodePtr Parser::parseVarDef() {
    auto ident = next_ident();
    auto dimensions = ast::ASTNodePtrs();
    bool has_impl = false;
    while (current != tokens.end()) {
        if (check(next_expect_delimiter(";"))) {
            previous();
            break;
        }
        if (check(next_expect_op("="))) {
            has_impl = true;
            break;
        }
        next_expect_delimiter("[");
        dimensions.push_back(parseConstExp());
        next_expect_delimiter("]");
    }
    if (has_impl) {
        auto initVal = parseInitVal();
        return uni<ast::VarDef>(ident->v, std::move(dimensions),
                                std::move(initVal));
    } else {
        return uni<ast::VarDef>(ident->v, std::move(dimensions));
    }
};

ast::ASTNodePtr Parser::parseInitVal() {
    if (next_expect_delimiter("{")) {
        auto initVals = ast::ASTNodePtrs();
        while (current != tokens.end()) {
            if (check(next_expect_delimiter("}"))) {
                break;
            }
            initVals.push_back(parseInitVal());
            if (check(next_expect_delimiter(","))) {
                continue;
            }
        }
        return uni<ast::InitVal>(std::move(initVals));
    } else {
        previous();
        return parseExp();
    }
}

ast::ASTNodePtr Parser::parseFuncDef() {
    auto funcType = next_tok<token::Ident>(Token::Type::Identifier);
    ast::ASTNode::FuncType funcTypeEnum;
    if (funcType->v == "void") {
        funcTypeEnum = ast::ASTNode::FuncType::Void;
    } else if (funcType->v == "int") {
        funcTypeEnum = ast::ASTNode::FuncType::Int;
    } else if (funcType->v == "float") {
        funcTypeEnum = ast::ASTNode::FuncType::Float;
    } else {
        throw unexpectedTokenError(funcType, "void, int or float");
    }
    auto ident = next_ident();
    next_expect_delimiter("(");
    auto funcFParams = parseFuncFParams();
    next_expect_delimiter(")");
    auto block = parseBlock();
    return uni<ast::FuncDef>(std::move(funcTypeEnum), ident->v,
                             std::move(funcFParams), std::move(block));
}

ast::ASTNodePtr Parser::parseFuncFParams() {
    auto funcFParams = ast::ASTNodePtrs();
    funcFParams.push_back(parseFuncFParam());
    while (current != tokens.end()) {
        if (check(next_expect_delimiter(")"))) {
            previous();
            break;
        }
        auto funcFParam = parseFuncFParam();
        next_expect_delimiter(",");
    }
    return uni<ast::FuncFParams>(std::move(funcFParams));
}
} // namespace frontend