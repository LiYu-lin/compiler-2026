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

#include "AST.h"
#include "Lexer.h"
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

namespace frontend {

template <typename L, typename R> struct tuple_cat_result {
    using type = std::tuple<L, R>;
};

template <typename... Ls, typename... Rs>
struct tuple_cat_result<std::tuple<Ls...>, std::tuple<Rs...>> {
    using type = std::tuple<Ls..., Rs...>;
};

template <typename... Ls, typename R>
struct tuple_cat_result<std::tuple<Ls...>, R> {
    using type = std::tuple<Ls..., R>;
};

template <typename L, typename... Rs>
struct tuple_cat_result<L, std::tuple<Rs...>> {
    using type = std::tuple<L, Rs...>;
};

// Helper alias template
template <typename L, typename R>
using tuple_cat_t = typename tuple_cat_result<L, R>::type;

template <typename T, typename Error> class Parsec;
struct ParserError;

template <typename T> using Parser = Parsec<T, ParserError>;

struct ParserError {
    std::string message;
    TokenPtrIterator state;
    ParserError(std::string message, TokenPtrIterator state)
        : message(message), state(state) {}

    std::string toString() const {
        return "ParserError: " + message + " at " +
               std::to_string(state->line) + ":" +
               std::to_string(state->column) + " - " + state->toString();
    }

    static ParserError endOfFile(TokenPtrIterator state) {
        return ParserError("End of file", state);
    }

    static ParserError unexpectedToken(TokenPtrIterator state) {
        return ParserError("Unexpected token", state);
    }

    static ParserError unexpectedToken(TokenPtrIterator state,
                                       const std::string &expected) {
        return ParserError("Unexpected token, expected " + expected, state);
    }
};

/**
 * @brief A class representing a pure value returned by the parser.
 *
 * @tparam U The type of the value returned by the parser.
 */
template <typename U> struct Pure {
    TokenPtrIterator state;
    U value;
    Pure(TokenPtrIterator state, U value) : state(state), value(value) {}
};

/**
 * @brief A class representing a parser combinator.
 *
 * @tparam T The type of the value returned by the parser.
 * @tparam Error The type of the error returned by the parser.
 */
template <typename T, typename Error> class Parsec {
public:
    using StateFunc =
        std::function<std::variant<Error, Pure<T>>(TokenPtrIterator)>;

    StateFunc stateFunc;

    Parsec() = default;

    /**
     * @brief Constructs a parser with the given state function.
     *
     * @param stateFunc The state function to be used by the parser.
     */
    Parsec(StateFunc stateFunc) : stateFunc(stateFunc) {}

    // Add a static method for lazy evaluation of recursive parsers
    static Parser<T> lazy(std::function<Parser<T>()> parserThunk) {
        return Parser<T>([parserThunk](TokenPtrIterator state)
                             -> std::variant<Error, Pure<T>> {
            // Only construct the actual parser when needed
            Parser<T> parser = parserThunk();
            return parser.stateFunc(state);
        });
    }

    /**
     * @brief Runs the parser on the given state.
     *
     * @param state The initial state of the parser.
     * @return T The value returned by the parser.
     */
    T run(TokenPtrIterator state) {
        auto result = stateFunc(state);
        if (std::holds_alternative<Error>(result)) {
            throw std::get<Error>(result);
        } else {
            auto pure = std::get<Pure<T>>(result);
            if (*pure.state != nullptr) {
                throw ParserError::unexpectedToken(pure.state);
            }
            return pure.value;
        }
    }

    template <typename U> Parser<U> bind(std::function<Parser<U>(T)> func) {
        return Parser<U>([p = *this, func](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<U>> {
            auto result = p.stateFunc(state);
            if (std::holds_alternative<ParserError>(result)) {
                return std::get<ParserError>(result);
            } else {
                auto pure = std::get<Pure<T>>(result);
                return func(pure.value).stateFunc(pure.state);
            }
        });
    }

    template <typename U> Parser<U> then(Parser<U> rhs) {
        return Parser<U>([*this, rhs](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<U>> {
            auto result = this->stateFunc(state);
            if (std::holds_alternative<ParserError>(result)) {
                return std::get<ParserError>(result);
            } else {
                auto pure = std::get<Pure<T>>(result);
                return rhs.stateFunc(pure.state);
            }
        });
    }

    template <typename U> Parser<T> with(Parser<U> rhs) {
        return Parser<T>([*this, rhs](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<T>> {
            auto result = this->stateFunc(state);
            if (std::holds_alternative<ParserError>(result)) {
                return std::get<ParserError>(result);
            } else {
                auto pure = std::get<Pure<T>>(result);
                auto rhs_result = rhs.stateFunc(pure.state);
                if (std::holds_alternative<ParserError>(rhs_result)) {
                    return std::get<ParserError>(rhs_result);
                } else {
                    return Pure<T>(std::get<Pure<U>>(rhs_result).state,
                                   pure.value);
                }
            }
        });
    }

    template <typename U> Parser<U> or_(Parser<U> rhs) {
        return Parser<U>([*this, rhs](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<U>> {
            auto result = this->stateFunc(state);
            if (std::holds_alternative<ParserError>(result)) {
                return rhs.stateFunc(state);
            } else {
                return result;
            }
        });
    }

    template <typename U> Parser<U> map(std::function<U(T)> func) {
        return Parser<U>([*this, func](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<U>> {
            auto result = this->stateFunc(state);
            if (std::holds_alternative<ParserError>(result)) {
                return std::get<ParserError>(result);
            } else {
                auto pure = std::get<Pure<T>>(result);
                return Pure<U>(pure.state, func(pure.value));
            }
        });
    }

    template <typename U> Parser<U> dyn_cast() {
        return this->map<U>(
            [](T value) { return std::dynamic_pointer_cast<U>(value); });
    }

    template <typename U> Parser<std::tuple<T, U>> and_(Parser<U> rhs) {
        return Parser<std::tuple<T, U>>(
            [*this, rhs](TokenPtrIterator state)
                -> std::variant<ParserError, Pure<std::tuple<T, U>>> {
                auto result = this->stateFunc(state);
                if (std::holds_alternative<ParserError>(result)) {
                    return std::get<ParserError>(result);
                } else {
                    auto pure = std::get<Pure<T>>(result);
                    auto rhs_result = rhs.stateFunc(pure.state);
                    if (std::holds_alternative<ParserError>(rhs_result)) {
                        return std::get<ParserError>(rhs_result);
                    } else {
                        auto rhs_pure = std::get<Pure<U>>(rhs_result);
                        return Pure<std::tuple<T, U>>(
                            rhs_pure.state,
                            std::make_tuple(pure.value, rhs_pure.value));
                    }
                }
            });
    }

    template <typename... U>
    Parser<std::tuple<T, U...>> extend(Parser<std::tuple<U...>> rhs) {
        return Parser<std::tuple<T, U...>>(
            [*this, rhs](TokenPtrIterator state)
                -> std::variant<ParserError, Pure<std::tuple<T, U...>>> {
                auto result = this->stateFunc(state);
                if (std::holds_alternative<ParserError>(result)) {
                    return std::get<ParserError>(result);
                } else {
                    auto pure = std::get<Pure<T>>(result);
                    auto rhs_result = rhs.stateFunc(pure.state);
                    if (std::holds_alternative<ParserError>(rhs_result)) {
                        return std::get<ParserError>(rhs_result);
                    } else {
                        auto rhs_pure =
                            std::get<Pure<std::tuple<U...>>>(rhs_result);
                        return Pure<std::tuple<T, U...>>(
                            rhs_pure.state,
                            std::tuple_cat(std::make_tuple(pure.value),
                                           rhs_pure.value));
                    }
                }
            });
    }

    Parser<std::optional<T>> opt() {
        return Parser<std::optional<T>>(
            [*this](TokenPtrIterator state)
                -> std::variant<ParserError, Pure<std::optional<T>>> {
                auto result = this->stateFunc(state);
                if (std::holds_alternative<ParserError>(result)) {
                    return Pure<std::optional<T>>(state, std::nullopt);
                } else {
                    auto pure = std::get<Pure<T>>(result);
                    return Pure<std::optional<T>>(
                        pure.state, std::make_optional(pure.value));
                }
            });
    }

    Parser<T> is(T value) {
        return Parser<T>([*this, value](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<T>> {
            auto result = this->stateFunc(state);
            if (std::holds_alternative<ParserError>(result)) {
                return std::get<ParserError>(result);
            } else {
                auto pure = std::get<Pure<T>>(result);
                if (pure.value == value) {
                    return pure;
                } else {
                    return ParserError::unexpectedToken(pure.state);
                }
            }
        });
    }

    Parser<T> not_(std::set<T> &values) {
        return Parser<T>([*this, values](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<T>> {
            auto result = this->stateFunc(state);
            if (std::holds_alternative<ParserError>(result)) {
                return std::get<ParserError>(result);
            } else {
                auto pure = std::get<Pure<T>>(result);
                if (values.count(pure.value) == 0) {
                    return pure;
                } else {
                    return ParserError::unexpectedToken(pure.state);
                }
            }
        });
    }

    // Parse zero or more occurrences of this parser
    Parser<std::vector<T>> many() {
        return Parser<std::vector<T>>(
            [*this](TokenPtrIterator state)
                -> std::variant<ParserError, Pure<std::vector<T>>> {
                std::vector<T> results;
                TokenPtrIterator current = state;

                while (true) {
                    auto result = this->stateFunc(current);
                    if (std::holds_alternative<ParserError>(result)) {
                        break;
                    } else {
                        auto pure = std::get<Pure<T>>(result);
                        results.push_back(pure.value);
                        current = pure.state;
                    }
                }

                return Pure<std::vector<T>>(current, results);
            });
    }

    // Parse one or more occurrences of this parser
    Parser<std::vector<T>> many1() {
        return Parser<std::vector<T>>(
            [*this](TokenPtrIterator state)
                -> std::variant<ParserError, Pure<std::vector<T>>> {
                auto first_result = this->stateFunc(state);
                if (std::holds_alternative<ParserError>(first_result)) {
                    return std::get<ParserError>(first_result);
                }

                auto first_pure = std::get<Pure<T>>(first_result);
                std::vector<T> results = {first_pure.value};
                TokenPtrIterator current = first_pure.state;

                while (true) {
                    if (*current == nullptr) {
                        break; // End of input
                    }
                    auto result = this->stateFunc(current);
                    if (std::holds_alternative<ParserError>(result)) {
                        break;
                    } else {
                        auto pure = std::get<Pure<T>>(result);
                        results.push_back(pure.value);
                        current = pure.state;
                    }
                }

                return Pure<std::vector<T>>(current, results);
            });
    }

    // Parse zero or more occurrences of this parser, separated by the separator
    // parser
    template <typename Sep> Parser<std::vector<T>> sep(Parser<Sep> separator) {
        return Parser<std::vector<T>>(
            [*this, separator](TokenPtrIterator state)
                -> std::variant<ParserError, Pure<std::vector<T>>> {
                std::vector<T> results;
                TokenPtrIterator current = state;

                // Try to parse the first item (optional)
                auto first_result = this->stateFunc(current);
                if (std::holds_alternative<ParserError>(first_result)) {
                    return Pure<std::vector<T>>(current,
                                                results); // Empty result
                }

                auto first_pure = std::get<Pure<T>>(first_result);
                results.push_back(first_pure.value);
                current = first_pure.state;

                while (true) {
                    // Try to parse separator
                    auto sep_result = separator.stateFunc(current);
                    if (std::holds_alternative<ParserError>(sep_result)) {
                        break;
                    }

                    auto sep_pure = std::get<Pure<Sep>>(sep_result);
                    current = sep_pure.state;

                    // Try to parse next item
                    auto item_result = this->stateFunc(current);
                    if (std::holds_alternative<ParserError>(item_result)) {
                        break;
                    }

                    auto item_pure = std::get<Pure<T>>(item_result);
                    results.push_back(item_pure.value);
                    current = item_pure.state;
                }

                return Pure<std::vector<T>>(current, results);
            });
    }

    // Parse one or more occurrences of this parser, separated by the separator
    // parser
    template <typename Sep> Parser<std::vector<T>> sep1(Parser<Sep> separator) {
        return Parser<std::vector<T>>(
            [*this, separator](TokenPtrIterator state)
                -> std::variant<ParserError, Pure<std::vector<T>>> {
                std::vector<T> results;
                TokenPtrIterator current = state;

                // Parse the first item (required)
                auto first_result = this->stateFunc(current);
                if (std::holds_alternative<ParserError>(first_result)) {
                    return std::get<ParserError>(
                        first_result); // Error if first item fails
                }

                auto first_pure = std::get<Pure<T>>(first_result);
                results.push_back(first_pure.value);
                current = first_pure.state;

                while (true) {
                    // Try to parse separator
                    auto sep_result = separator.stateFunc(current);
                    if (std::holds_alternative<ParserError>(sep_result)) {
                        break;
                    }

                    auto sep_pure = std::get<Pure<Sep>>(sep_result);
                    current = sep_pure.state;

                    // Try to parse next item
                    auto item_result = this->stateFunc(current);
                    if (std::holds_alternative<ParserError>(item_result)) {
                        break;
                    }

                    auto item_pure = std::get<Pure<T>>(item_result);
                    results.push_back(item_pure.value);
                    current = item_pure.state;
                }

                return Pure<std::vector<T>>(current, results);
            });
    }

    Parser<T> try_() {
        return Parser<T>([*this](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<T>> {
            auto state_ = state;
            auto result = this->stateFunc(state);
            if (std::holds_alternative<ParserError>(result)) {
                return std::get<ParserError>(result);
            } else {
                return result; // Return the successful result
            }
        });
    }

template <typename Op>
    Parser<T> chain(Parser<Op> opParser, std::function<T(Op, T, T)> combine) {
        return Parser<T>([*this, opParser, combine](TokenPtrIterator state)
                             -> std::variant<ParserError, Pure<T>> {
            // 1. 解析第一个左侧项
            auto firstResult = this->stateFunc(state);
            if (std::holds_alternative<ParserError>(firstResult)) {
                return std::get<ParserError>(firstResult);
            }

            auto currentPure = std::get<Pure<T>>(firstResult);
            T leftValue = currentPure.value;
            TokenPtrIterator currentState = currentPure.state;

            // 2. 循环尝试解析操作符和右侧项
            while (true) {
                // 核心：每次循环前记录当前状态快照，用于右项失败时完整回溯
                TokenPtrIterator backtrackState = currentState;

                // 尝试解析操作符
                auto opResult = opParser.stateFunc(currentState);
                if (std::holds_alternative<ParserError>(opResult)) {
                    break; // 没有匹配到操作符，正常结束 chain
                }
                auto opPure = std::get<Pure<Op>>(opResult);
                Op op = opPure.value;

                // 尝试解析右侧项
                auto rightResult = this->stateFunc(opPure.state);
                if (std::holds_alternative<ParserError>(rightResult)) {
                    // 如果右侧项解析失败，证明这个操作符属于更外层的语法成分
                    // 必须把状态完整退回到解析该操作符之前！
                    currentState = backtrackState;
                    break;
                }

                auto rightPure = std::get<Pure<T>>(rightResult);
                T rightValue = rightPure.value;

                // 成功匹配一组，组合左右项，并更新当前状态指针
                leftValue = combine(op, leftValue, rightValue);
                currentState = rightPure.state;
            }

            return Pure<T>(currentState, leftValue);
        });
    }
};

namespace parser {

inline Parser<TokenPtr> next = Parser<TokenPtr>(
    [](TokenPtrIterator state) -> std::variant<ParserError, Pure<TokenPtr>> {
        if (*state == nullptr) {
            return ParserError::endOfFile(state);
        } else {
            return Pure(state + 1, *state);
        }
    });

inline Parser<TokenPtr> satisfy(std::function<bool(TokenPtr)> predicate) {
    return Parser<TokenPtr>([predicate](TokenPtrIterator state)
                                -> std::variant<ParserError, Pure<TokenPtr>> {
        if (*state == nullptr) {
            return ParserError::endOfFile(state);
        } else if (predicate(*state)) {
            return Pure(state + 1, *state);
        } else {
            return ParserError::unexpectedToken(state);
        }
    });
}

inline Parser<std::string> str =
    satisfy([](TokenPtr token) {
        return token->type == Token::Type::Identifier;
    }).map<std::string>([](TokenPtr token) {
        return std::dynamic_pointer_cast<token::Ident>(token)->v;
    });

inline Parser<std::string> sym =
    satisfy([](TokenPtr token) {
        return token->type == Token::Type::Operator ||
               token->type == Token::Type::Delimiter;
    }).map<std::string>([](TokenPtr token) {
        std::string str;
        if (token->type == Token::Type::Operator) {
            str = std::dynamic_pointer_cast<token::Operator>(token)->v;
        } else {
            str = std::dynamic_pointer_cast<token::Delimiter>(token)->v;
        }
        return str;
    });

inline Parser<std::string> symbol(const std::string &s) {
    return Parser<std::string>(
        [s](TokenPtrIterator state)
            -> std::variant<ParserError, Pure<std::string>> {
            if (*state == nullptr) {
                return ParserError::endOfFile(state);
            } else if ((*state)->type == Token::Type::Operator) {
                auto op = std::dynamic_pointer_cast<token::Operator>(*state);
                if (op->v == s) {
                    return Pure(state + 1, s);
                }
            } else if ((*state)->type == Token::Type::Delimiter) {
                auto delim =
                    std::dynamic_pointer_cast<token::Delimiter>(*state);
                if (delim->v == s) {
                    return Pure(state + 1, s);
                }
            } else if ((*state)->type == Token::Type::Identifier) {
                auto reserved = std::dynamic_pointer_cast<token::Ident>(*state);
                if (reserved->v == s) {
                    return Pure(state + 1, s);
                }
            }
            return ParserError::unexpectedToken(state, s);
        });
}

inline Parser<std::string> ident = str.not_(Token::reserved);

inline Parser<ast::ASTNode::BType> btype =
    str.is("int")
        .map<ast::ASTNode::BType>(
            [](std::string) { return ast::ASTNode::BType::Int; })
        .or_(str.is("float").map<ast::ASTNode::BType>(
            [](std::string) { return ast::ASTNode::BType::Float; }));

inline Parser<ast::ASTNode::FuncType> functype =
    str.is("int")
        .map<ast::ASTNode::FuncType>(
            [](std::string) { return ast::ASTNode::FuncType::Int; })
        .or_(str.is("float").map<ast::ASTNode::FuncType>(
            [](std::string) { return ast::ASTNode::FuncType::Float; }))
        .or_(str.is("void").map<ast::ASTNode::FuncType>(
            [](std::string) { return ast::ASTNode::FuncType::Void; }));

inline Parser<ast::Number> number =
    satisfy([](TokenPtr token) {
        return token->type == Token::Type::FloatConst ||
               token->type == Token::Type::IntConst;
    }).map<ast::Number>([](TokenPtr token) -> ast::Number {
        if (token->type == Token::Type::FloatConst) {
            return ast::Number(
                std::dynamic_pointer_cast<token::FloatConst>(token)->v);
        } else {
            return ast::Number(
                std::dynamic_pointer_cast<token::IntConst>(token)->v);
        }
    });

inline Parser<TokenPtrIterator> getState =
    Parser<TokenPtrIterator>([](TokenPtrIterator state) {
        return Pure<TokenPtrIterator>(state, state);
    });

inline Parser<TokenPtrIterator> setState(TokenPtrIterator state) {
    return Parser<TokenPtrIterator>([state](TokenPtrIterator) {
        return Pure<TokenPtrIterator>(state, state);
    });
}
template <typename T> inline Parser<T> pure(T value) {
    return Parser<T>(
        [value](TokenPtrIterator state) { return Pure<T>(state, value); });
}

template <typename T> inline Parser<T> fail(ParserError error) {
    return Parser<T>([error](TokenPtrIterator state) { return error; });
}

} // namespace parser
} // namespace frontend
