#include "../../include/Parse/Grammar.h"
#include "../../include/Parse/Lexer.h"
#include "../../include/Parse/Parser.h"

namespace frontend {
namespace parser {

Parser<ast::ASTNodePtr> comp_unit() {
    return (func_def().or_(decl().with(symbol(";"))))
        .many1()
        .map<ast::ASTNodePtr>(
            [](auto decls) { return box<ast::CompUnit>(decls); });
}

Parser<ast::ASTNodePtr> decl() {
    return const_decl().or_(var_decl()).map<ast::ASTNodePtr>([](auto t) {
        return box<ast::Decl>(t);
    });
}

Parser<ast::ASTNodePtr> const_decl() {
    return str.is("const")
        .then(btype)
        .and_(const_def().sep1(symbol(",")))
        .map<ast::ASTNodePtr>([](auto t) {
            return box<ast::ConstDecl>(std::get<0>(t), std::get<1>(t));
        });
}

Parser<ast::ASTNodePtr> const_def() {
    return ident
        .extend(symbol("[")
                    .then(const_exp())
                    .with(symbol("]"))
                    .many()
                    .and_(symbol("=").then(const_init_val())))
        .map<ast::ASTNodePtr>([](auto t) {
            auto id = std::get<0>(t);
            auto arr = std::get<1>(t);
            auto init = std::get<2>(t);
            return box<ast::ConstDef>(id, arr, init);
        });
}

Parser<ast::ASTNodePtr> const_init_val() {
    return symbol("{")
        .then(Parser<std::vector<ast::ASTNodePtr>>::lazy(
            []() { return const_init_val().sep(symbol(",")); }))
        .with(symbol("}"))
        .map<ast::ASTNodePtr>([](auto t) { return box<ast::ConstInitVal>(t); })
        .or_(const_exp().map<ast::ASTNodePtr>(
            [](auto t) { return box<ast::ConstInitVal>(t); }));
}

Parser<ast::ASTNodePtr> var_decl() {
    return btype.and_(var_def().sep1(symbol(",")))
        .map<ast::ASTNodePtr>([](auto t) {
            return box<ast::VarDecl>(std::get<0>(t), std::get<1>(t));
        });
}

Parser<ast::ASTNodePtr> var_def() {
    return ident
        .extend(symbol("[")
                    .then(const_exp())
                    .with(symbol("]"))
                    .many()
                    .and_(symbol("=").then(init_val()).opt()))
        .map<ast::ASTNodePtr>([](auto t) {
            auto id = std::get<0>(t);
            auto arr = std::get<1>(t);
            auto init = std::get<2>(t);
            if (init == std::nullopt) {
                return box<ast::VarDef>(id, arr);
            }
            return box<ast::VarDef>(id, arr, init);
        });
}

Parser<ast::ASTNodePtr> init_val() {
    return symbol("{")
        .then(Parser<std::vector<ast::ASTNodePtr>>::lazy(
            []() { return init_val().sep(symbol(",")); }))
        .with(symbol("}"))
        .map<ast::ASTNodePtr>([](auto t) { return box<ast::InitVal>(t); })
        .or_(Parser<ast::ASTNodePtr>::lazy([]() {
            return expr().map<ast::ASTNodePtr>(
                [](auto t) { return box<ast::InitVal>(t); });
        }));
}

Parser<ast::ASTNodePtr> func_def() {
    return functype
        .extend(ident.extend(
            symbol("(").then(func_fparams()).with(symbol(")")).and_(block())))
        .map<ast::ASTNodePtr>([](auto t) {
            auto type = std::get<0>(t);
            auto id = std::get<1>(t);
            auto params = std::get<2>(t);
            auto block = std::get<3>(t);
            return box<ast::FuncDef>(type, id, params, block);
        });
}

Parser<ast::ASTNodePtr> func_fparams() {
    return func_fparam().sep(symbol(",")).map<ast::ASTNodePtr>([](auto t) {
        return box<ast::FuncFParams>(t);
    });
}

Parser<ast::ASTNodePtr> func_fparam() {
    return btype
        .extend(ident.and_(symbol("[")
                               .then(symbol("]"))
                               .then(symbol("[")
                                         .then(Parser<ast::ASTNodePtr>::lazy(
                                             []() { return expr(); }))
                                         .with(symbol("]"))
                                         .many())
                               .opt()))
        .map<ast::ASTNodePtr>([](auto t) {
            auto type = std::get<0>(t);
            auto id = std::get<1>(t);
            auto dim = std::get<2>(t);
            return box<ast::FuncFParam>(type, id, dim);
        });
}

Parser<ast::ASTNodePtr> block() {
    return symbol("{")
        .then(
            Parser<ast::ASTNodePtr>::lazy([]() { return block_item(); }).many())
        .with(symbol("}"))
        .map<ast::ASTNodePtr>([](auto t) { return box<ast::Block>(t); });
}

Parser<ast::ASTNodePtr> block_item() {
    return decl()
        .or_(Parser<ast::ASTNodePtr>::lazy([]() { return stmt(); }))
        .map<ast::ASTNodePtr>([](auto t) { return box<ast::BlockItem>(t); });
}

Parser<ast::ASTNodePtr> stmt_assign() {
    return lval().and_(symbol("=")).try_()
        .and_(Parser<ast::ASTNodePtr>::lazy([]() { return expr(); }))
        .with(symbol(";"))
        .map<ast::ASTNodePtr>([](auto t) {
            auto left_pair = std::get<0>(t); 
            auto lval = std::get<0>(left_pair); 
            auto exp = std::get<1>(t);          
            return box<ast::Stmt::AssignStmt>(lval, exp);
        });
}

Parser<ast::ASTNodePtr> stmt_exp() {
    return Parser<ast::ASTNodePtr>::lazy([]() { return expr(); })
        .opt()
        .with(symbol(";"))
        .map<ast::ASTNodePtr>(
            [](auto t) { return box<ast::Stmt::ExpStmt>(t); });
}

Parser<ast::ASTNodePtr> stmt_block() {
    return block().map<ast::ASTNodePtr>(
        [](auto t) { return box<ast::Stmt::BlockStmt>(t); });
}

Parser<ast::ASTNodePtr> stmt_if() {
    return symbol("if")
        .then(symbol("("))
        .then(cond())
        .with(symbol(")"))
        .extend(Parser<ast::ASTNodePtr>::lazy([]() { return stmt(); })
                    .and_(symbol("else")
                              .then(Parser<ast::ASTNodePtr>::lazy([]() { return stmt(); }))
                              .opt()))
        .map<ast::ASTNodePtr>([](auto t) {
            auto cond = std::get<0>(t);
            auto block = std::get<1>(t);
            auto else_block = std::get<2>(t);
            return box<ast::Stmt::IfStmt>(cond, block, else_block);
        });
}

Parser<ast::ASTNodePtr> stmt_while() {
    return symbol("while")
        .then(symbol("("))
        .then(cond())
        .with(symbol(")"))
        .and_(Parser<ast::ASTNodePtr>::lazy([]() { return stmt(); }))
        .map<ast::ASTNodePtr>([](auto t) {
            auto cond = std::get<0>(t);
            auto stmt = std::get<1>(t);
            return box<ast::Stmt::WhileStmt>(cond, stmt);
        });
}

Parser<ast::ASTNodePtr> stmt_break() {
    return symbol("break").with(symbol(";")).map<ast::ASTNodePtr>([](auto) {
        return box<ast::Stmt::BreakStmt>();
    });
}

Parser<ast::ASTNodePtr> stmt_continue() {
    return symbol("continue").with(symbol(";")).map<ast::ASTNodePtr>([](auto) {
        return box<ast::Stmt::ContinueStmt>();
    });
}

Parser<ast::ASTNodePtr> stmt_return() {
    return symbol("return")
        .then(Parser<ast::ASTNodePtr>::lazy([]() { return expr(); }).opt())
        .with(symbol(";"))
        .map<ast::ASTNodePtr>(
            [](auto t) { return box<ast::Stmt::ReturnStmt>(t); });
}

Parser<ast::ASTNodePtr> stmt() {
    auto keywordInterceptor = Parser<ast::ASTNodePtr>([](TokenPtrIterator state) 
        -> std::variant<ParserError, Pure<ast::ASTNodePtr>> {
            if (*state != nullptr && (*state)->type == frontend::Token::Type::Identifier) {
                auto idTok = std::dynamic_pointer_cast<frontend::token::Ident>(*state);
                if (idTok && frontend::Token::reserved.count(idTok->v) > 0) {
                    return ParserError("Fatal: Syntax error inside keyword block of '" + idTok->v + "'", state);
                }
            }
            return ParserError::unexpectedToken(state);
        });

    return stmt_block()
        .or_(stmt_if())
        .or_(stmt_while())
        .or_(stmt_break())
        .or_(stmt_continue())
        .or_(stmt_return())
        .or_(keywordInterceptor)
        .or_(stmt_assign())
        .or_(stmt_exp())
        .map<ast::ASTNodePtr>([](auto t) { return box<ast::Stmt>(t); });
}

Parser<ast::ASTNodePtr> expr() {
    // 算术表达式只能到 add_exp 结束，绝对不能混入关系运算符和逻辑短路符！
    return add_exp().map<ast::ASTNodePtr>(
        [](auto t) { return box<ast::Exp>(t); });
}

Parser<ast::ASTNodePtr> cond() {
    return l_or_exp().map<ast::ASTNodePtr>(
        [](auto t) { return box<ast::Cond>(t); });
}

Parser<ast::ASTNodePtr> lval() {
    return ident
        .and_(symbol("[")
                  .then(Parser<ast::ASTNodePtr>::lazy([]() { return expr(); }))
                  .with(symbol("]"))
                  .many())
        .map<ast::ASTNodePtr>([](auto t) {
            auto id = std::get<0>(t);
            auto indices = std::get<1>(t);
            return box<ast::LVal>(id, indices);
        });
}

Parser<ast::ASTNodePtr> primary_exp() {
    return number
        .map<ast::ASTNodePtr>([](auto t) { return box<ast::PrimaryExp>(t); })
        .or_(symbol("(")
                 // 核心修正：括号内不仅可以是一个普通的算术 expr，更可以是一个完整的控制流 cond！
                 // 我们用 cond() 代替 expr()，从而让括号内部能够完美规约所有的 && 和 || 符号！
                 .then(Parser<ast::ASTNodePtr>::lazy([]() { return cond(); }))
                 .with(symbol(")"))
                 .map<ast::ASTNodePtr>(
                     [](auto t) { return box<ast::PrimaryExp>(t); }))
        .or_(Parser<ast::ASTNodePtr>::lazy([]() {
            return lval().map<ast::ASTNodePtr>(
                [](auto t) { return box<ast::PrimaryExp>(t); });
        }));
}

Parser<ast::ASTNodePtr> unary_exp_call() {
    return ident
        .and_(symbol("(")
                  .then(Parser<std::vector<ast::ASTNodePtr>>::lazy(
                      []() { return expr().sep(symbol(",")); }))
                  .with(symbol(")")))
        .map<ast::ASTNodePtr>([](auto t) {
            auto id = std::get<0>(t);
            auto params = std::get<1>(t);
            return box<ast::UnaryExp::UnaryExpCall>(id, params);
        });
}

Parser<ast::ASTNodePtr> unary_exp_op() {
    return getState
        .bind<ast::ASTNode::UnaryOp>(
            [](auto state) -> Parser<ast::ASTNode::UnaryOp> {
                return sym.bind<ast::ASTNode::UnaryOp>(
                    [state](std::string s) -> Parser<ast::ASTNode::UnaryOp> {
                        if (s == "+")
                            return pure(ast::ASTNode::UnaryOp::Plus);
                        if (s == "-")
                            return pure(ast::ASTNode::UnaryOp::Minus);
                        if (s == "!")
                            return pure(ast::ASTNode::UnaryOp::Not);
                        return fail<ast::ASTNode::UnaryOp>(
                            ParserError::unexpectedToken(state, "+ - !"));
                    });
            })
        .and_(Parser<ast::ASTNodePtr>::lazy(
            []() { return unary_exp(); })) 
        .map<ast::ASTNodePtr>([](auto t) {
            auto op = std::get<0>(t);
            auto exp = std::get<1>(t);
            return box<ast::UnaryExp::UnaryExpOp>(op, exp);
        });
}

Parser<ast::ASTNodePtr> unary_exp() {
    return unary_exp_op()
        .or_(unary_exp_call())
        .or_(primary_exp())
        .map<ast::ASTNodePtr>([](auto t) { return box<ast::UnaryExp>(t); });
}

Parser<ast::ASTNodePtr> mul_exp() {
    auto opParser = getState.bind<ast::ASTNode::MulOp>(
        [](auto state) -> Parser<ast::ASTNode::MulOp> {
            return sym.bind<ast::ASTNode::MulOp>(
                [state](std::string s) -> Parser<ast::ASTNode::MulOp> {
                    if (s == "*")
                        return pure(ast::ASTNode::MulOp::Mul);
                    if (s == "/")
                        return pure(ast::ASTNode::MulOp::Div);
                    if (s == "%")
                        return pure(ast::ASTNode::MulOp::Mod);
                    return fail<ast::ASTNode::MulOp>(
                        ParserError::unexpectedToken(state, "* / %"));
                });
        }).try_();

    return unary_exp().chain<ast::ASTNode::MulOp>(
        opParser,
        [](ast::ASTNode::MulOp op, ast::ASTNodePtr lhs, ast::ASTNodePtr rhs) {
            return box<ast::MulExp>(box<ast::MulExp::MulExpOp>(op, lhs, rhs));
        });
}

Parser<ast::ASTNodePtr> add_exp() {
    auto opParser = getState.bind<ast::ASTNode::AddOp>(
        [](auto state) -> Parser<ast::ASTNode::AddOp> {
            return sym.bind<ast::ASTNode::AddOp>(
                [state](std::string s) -> Parser<ast::ASTNode::AddOp> {
                    if (s == "+")
                        return pure(ast::ASTNode::AddOp::Plus);
                    if (s == "-")
                        return pure(ast::ASTNode::AddOp::Minus);
                    return fail<ast::ASTNode::AddOp>(
                        ParserError::unexpectedToken(state, "+ -"));
                });
        }).try_();

    return mul_exp().chain<ast::ASTNode::AddOp>(
        opParser,
        [](ast::ASTNode::AddOp op, ast::ASTNodePtr lhs, ast::ASTNodePtr rhs) {
            return box<ast::AddExp>(box<ast::AddExp::AddExpOp>(op, lhs, rhs));
        });
}

Parser<ast::ASTNodePtr> rel_exp() {
    auto opParser = getState.bind<ast::ASTNode::RelOp>(
        [](auto state) -> Parser<ast::ASTNode::RelOp> {
            return sym.bind<ast::ASTNode::RelOp>(
                [state](std::string s) -> Parser<ast::ASTNode::RelOp> {
                    if (s == "<")
                        return pure(ast::ASTNode::RelOp::Less);
                    if (s == "<=")
                        return pure(ast::ASTNode::RelOp::LessEqual);
                    if (s == ">")
                        return pure(ast::ASTNode::RelOp::Greater);
                    if (s == ">=")
                        return pure(ast::ASTNode::RelOp::GreaterEqual);
                    return fail<ast::ASTNode::RelOp>(
                        ParserError::unexpectedToken(state, "< <= > >="));
                });
        }).try_();

    return add_exp().chain<ast::ASTNode::RelOp>(
        opParser,
        [](ast::ASTNode::RelOp op, ast::ASTNodePtr lhs, ast::ASTNodePtr rhs) {
            return box<ast::RelExp>(box<ast::RelExp::RelExpOp>(op, lhs, rhs));
        });
}

Parser<ast::ASTNodePtr> eq_exp() {
    auto opParser = getState.bind<ast::ASTNode::EqOp>(
        [](auto state) -> Parser<ast::ASTNode::EqOp> {
            return sym.bind<ast::ASTNode::EqOp>(
                [state](std::string s) -> Parser<ast::ASTNode::EqOp> {
                    if (s == "==")
                        return pure(ast::ASTNode::EqOp::Equal);
                    if (s == "!=")
                        return pure(ast::ASTNode::EqOp::NotEqual);
                    return fail<ast::ASTNode::EqOp>(
                        ParserError::unexpectedToken(state, "== !="));
                });
        }).try_();

    return rel_exp().chain<ast::ASTNode::EqOp>(
        opParser,
        [](ast::ASTNode::EqOp op, ast::ASTNodePtr lhs, ast::ASTNodePtr rhs) {
            return box<ast::EqExp>(box<ast::EqExp::EqExpOp>(op, lhs, rhs));
        });
}

Parser<ast::ASTNodePtr> l_and_exp() {
    auto opParser = symbol("&&").map<bool>([](auto) { return true; }).try_();

    return eq_exp().chain<bool>(
        opParser, [](bool, ast::ASTNodePtr lhs, ast::ASTNodePtr rhs) {
            return box<ast::LAndExp>(box<ast::LAndExp::LAndExpOp>(lhs, rhs));
        });
}

Parser<ast::ASTNodePtr> l_or_exp() {
    auto opParser = symbol("||").map<bool>([](auto) { return true; }).try_();

    return l_and_exp().chain<bool>(
        opParser, [](bool, ast::ASTNodePtr lhs, ast::ASTNodePtr rhs) {
            return box<ast::LOrExp>(box<ast::LOrExp::LOrExpOp>(lhs, rhs));
        });
}

Parser<ast::ASTNodePtr> const_exp() {
    return add_exp().map<ast::ASTNodePtr>(
        [](auto t) { return box<ast::ConstExp>(t); });
}

} // namespace parser
} // namespace frontend