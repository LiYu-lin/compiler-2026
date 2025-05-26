/**
 * @file Grammar.h
 * @author Dexer Matters (dexermatters@gmail.com)
 * @brief A file containing the definition of the Grammar rules.
 * @version 0.1
 * @date 2025-05-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
#include "Parse/Parser.h"

namespace frontend {
namespace parser {
// Token parsers

Parser<ast::ASTNodePtr> comp_unit();
Parser<ast::ASTNodePtr> decl();
Parser<ast::ASTNodePtr> const_decl();
Parser<ast::ASTNodePtr> const_def();
Parser<ast::ASTNodePtr> const_init_val();
Parser<ast::ASTNodePtr> var_decl();
Parser<ast::ASTNodePtr> var_def();
Parser<ast::ASTNodePtr> init_val();
Parser<ast::ASTNodePtr> func_def();
Parser<ast::ASTNodePtr> func_fparams();
Parser<ast::ASTNodePtr> func_fparam();
Parser<ast::ASTNodePtr> block();
Parser<ast::ASTNodePtr> block_item();
Parser<ast::ASTNodePtr> stmt();
Parser<ast::ASTNodePtr> stmt_assign();
Parser<ast::ASTNodePtr> stmt_exp();
Parser<ast::ASTNodePtr> stmt_block();
Parser<ast::ASTNodePtr> stmt_if();
Parser<ast::ASTNodePtr> stmt_while();
Parser<ast::ASTNodePtr> stmt_break();
Parser<ast::ASTNodePtr> stmt_continue();
Parser<ast::ASTNodePtr> stmt_return();
Parser<ast::ASTNodePtr> expr();
Parser<ast::ASTNodePtr> cond();
Parser<ast::ASTNodePtr> lval();
Parser<ast::ASTNodePtr> primary_exp();
Parser<ast::ASTNodePtr> unary_exp();
Parser<ast::ASTNodePtr> unary_exp_call();
Parser<ast::ASTNodePtr> unary_exp_op();
Parser<ast::ASTNodePtr> mul_exp();
Parser<ast::ASTNodePtr> add_exp();
Parser<ast::ASTNodePtr> rel_exp();
Parser<ast::ASTNodePtr> eq_exp();
Parser<ast::ASTNodePtr> l_and_exp();
Parser<ast::ASTNodePtr> l_or_exp();
Parser<ast::ASTNodePtr> const_exp();

} // namespace parser
} // namespace frontend