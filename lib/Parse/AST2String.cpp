/**
 * @file AST2String.cpp
 * @author Dexer Matters (dexermatters@gmail.com)
 * @brief Implementation of the AST toString methods.
 * @version 0.1
 * @date 2025-05-17
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "Parse/AST.h"
#include <sstream>

namespace frontend {
namespace ast {

std::string CompUnit::toString() const {
    std::stringstream ss;
    ss << "CompUnit";
    if (!decls.empty()) {
        for (const auto &decl : decls) {
            ss << "\n" << makeIndent(2) << decl->toString();
        }
    }
    return ss.str();
}

std::string Decl::toString() const {
    std::stringstream ss;
    ss << "Decl\n" << makeIndent(2) << decl->toString();
    return ss.str();
}

std::string ConstDecl::toString() const {
    std::stringstream ss;
    ss << "ConstDecl";
    ss << "\n"
       << makeIndent(2) << "BType: " << (btype == BType::Int ? "int" : "float");
    if (!constDefs.empty()) {
        for (const auto &def : constDefs) {
            ss << "\n" << makeIndent(2) << def->toString();
        }
    }
    return ss.str();
}

std::string ConstDef::toString() const {
    std::stringstream ss;
    ss << "ConstDef: " << ident;
    if (!dimensions.empty()) {
        ss << "\n" << makeIndent(2) << "Dimensions:";
        for (const auto &dim : dimensions) {
            ss << "\n" << makeIndent(4) << dim->toString();
        }
    }
    if (constInitVal) {
        ss << "\n"
           << makeIndent(2) << "InitValue:\n"
           << makeIndent(4) << constInitVal->toString();
    }
    return ss.str();
}

std::string ConstInitVal::toString() const {
    std::stringstream ss;
    ss << "ConstInitVal";
    if (std::holds_alternative<ASTNodePtr>(constInitVal)) {
        ss << "\n"
           << makeIndent(2) << std::get<ASTNodePtr>(constInitVal)->toString();
    } else if (std::holds_alternative<ASTNodePtrs>(constInitVal)) {
        const auto &vals = std::get<ASTNodePtrs>(constInitVal);
        if (!vals.empty()) {
            for (const auto &val : vals) {
                ss << "\n" << makeIndent(2) << val->toString();
            }
        }
    }
    return ss.str();
}

std::string VarDecl::toString() const {
    std::stringstream ss;
    ss << "VarDecl";
    ss << "\n"
       << makeIndent(2) << "BType: " << (btype == BType::Int ? "int" : "float");
    if (!varDefs.empty()) {
        for (const auto &def : varDefs) {
            ss << "\n" << makeIndent(2) << def->toString();
        }
    }
    return ss.str();
}

std::string VarDef::toString() const {
    std::stringstream ss;
    ss << "VarDef: " << ident;
    if (!dimensions.empty()) {
        ss << "\n" << makeIndent(2) << "Dimensions:";
        for (const auto &dim : dimensions) {
            ss << "\n" << makeIndent(4) << dim->toString();
        }
    }
    if (initVal && *initVal) {
        ss << "\n"
           << makeIndent(2) << "InitValue:\n"
           << makeIndent(4) << (*initVal)->toString();
    }
    return ss.str();
}

std::string InitVal::toString() const {
    std::stringstream ss;
    ss << "InitVal";
    if (std::holds_alternative<ASTNodePtr>(initVal)) {
        ss << "\n"
           << makeIndent(2) << std::get<ASTNodePtr>(initVal)->toString();
    } else if (std::holds_alternative<ASTNodePtrs>(initVal)) {
        const auto &vals = std::get<ASTNodePtrs>(initVal);
        if (!vals.empty()) {
            for (const auto &val : vals) {
                ss << "\n" << makeIndent(2) << val->toString();
            }
        }
    }
    return ss.str();
}

std::string FuncDef::toString() const {
    std::stringstream ss;
    std::string typeStr;
    switch (funcType) {
    case FuncType::Void:
        typeStr = "void";
        break;
    case FuncType::Int:
        typeStr = "int";
        break;
    case FuncType::Float:
        typeStr = "float";
        break;
    }
    ss << "FuncDef: " << ident << " -> " << typeStr;
    ss << "\n"
       << makeIndent(2) << "Params:\n"
       << makeIndent(4) << funcFParams->toString();
    ss << "\n"
       << makeIndent(2) << "Body:\n"
       << makeIndent(4) << block->toString();
    return ss.str();
}

std::string FuncFParams::toString() const {
    std::stringstream ss;
    ss << "FuncFParams";
    if (!funcFParams.empty()) {
        for (const auto &param : funcFParams) {
            ss << "\n" << makeIndent(2) << param->toString();
        }
    }
    return ss.str();
}

std::string FuncFParam::toString() const {
    std::stringstream ss;
    ss << "FuncFParam: " << ident->toString() << " -> " << btype->toString();
    if (dimensions && !dimensions->empty()) {
        ss << "\n" << makeIndent(2) << "Dimensions:";
        for (const auto &dim : *dimensions) {
            ss << "\n" << makeIndent(4) << dim->toString();
        }
    }
    return ss.str();
}

std::string Block::toString() const {
    std::stringstream ss;
    ss << "Block";
    if (!blockItems.empty()) {
        for (const auto &item : blockItems) {
            ss << "\n" << makeIndent(2) << item->toString();
        }
    }
    return ss.str();
}

std::string BlockItem::toString() const {
    std::stringstream ss;
    ss << "BlockItem\n" << makeIndent(2) << item->toString();
    return ss.str();
}

std::string Stmt::AssignStmt::toString() const {
    std::stringstream ss;
    ss << "AssignStmt";
    ss << "\n"
       << makeIndent(2) << "LVal:\n"
       << makeIndent(4) << lval->toString();
    ss << "\n"
       << makeIndent(2) << "Expr:\n"
       << makeIndent(4) << exp->toString();
    return ss.str();
}

std::string Stmt::ExpStmt::toString() const {
    std::stringstream ss;
    ss << "ExpStmt";
    if (exp && *exp) {
        ss << "\n" << makeIndent(2) << (*exp)->toString();
    }
    return ss.str();
}

std::string Stmt::BlockStmt::toString() const {
    std::stringstream ss;
    ss << "BlockStmt\n" << makeIndent(2) << block->toString();
    return ss.str();
}

std::string Stmt::IfStmt::toString() const {
    std::stringstream ss;
    ss << "IfStmt";
    ss << "\n"
       << makeIndent(2) << "Condition:\n"
       << makeIndent(4) << exp->toString();
    ss << "\n"
       << makeIndent(2) << "Then:\n"
       << makeIndent(4) << block->toString();
    if (elseStmt && *elseStmt) {
        ss << "\n"
           << makeIndent(2) << "Else:\n"
           << makeIndent(4) << (*elseStmt)->toString();
    }
    return ss.str();
}

std::string Stmt::WhileStmt::toString() const {
    std::stringstream ss;
    ss << "WhileStmt";
    ss << "\n"
       << makeIndent(2) << "Condition:\n"
       << makeIndent(4) << cond->toString();
    ss << "\n"
       << makeIndent(2) << "Body:\n"
       << makeIndent(4) << stmt->toString();
    return ss.str();
}

std::string Stmt::BreakStmt::toString() const { return "BreakStmt"; }

std::string Stmt::ContinueStmt::toString() const { return "ContinueStmt"; }

std::string Stmt::ReturnStmt::toString() const {
    std::stringstream ss;
    ss << "ReturnStmt";
    if (exp && *exp) {
        ss << "\n" << makeIndent(2) << (*exp)->toString();
    }
    return ss.str();
}

std::string Stmt::toString() const {
    std::stringstream ss;
    ss << "Stmt\n" << makeIndent(2) << stmt->toString();
    return ss.str();
}

std::string LVal::toString() const {
    std::stringstream ss;
    ss << "LVal: " << ident->toString();
    if (!dimensions.empty()) {
        ss << "\n" << makeIndent(2) << "Dimensions:";
        for (const auto &dim : dimensions) {
            ss << "\n" << makeIndent(4) << dim->toString();
        }
    }
    return ss.str();
}

std::string PrimaryExp::toString() const {
    std::stringstream ss;
    ss << "PrimaryExp";
    if (std::holds_alternative<ASTNodePtr>(primaryExp)) {
        ss << "\n"
           << makeIndent(2) << std::get<ASTNodePtr>(primaryExp)->toString();
    } else if (std::holds_alternative<Number>(primaryExp)) {
        ss << ": " << std::get<Number>(primaryExp).toString();
    }
    return ss.str();
}

std::string UnaryExp::UnaryExpCall::toString() const {
    std::stringstream ss;
    ss << "FunctionCall: " << ident->toString();
    if (!funcRParams.empty()) {
        ss << "\n" << makeIndent(2) << "Args:";
        for (const auto &param : funcRParams) {
            ss << "\n" << makeIndent(4) << param->toString();
        }
    }
    return ss.str();
}

std::string UnaryExp::UnaryExpOp::toString() const {
    std::stringstream ss;
    ss << "UnaryOp: ";
    switch (unaryOp) {
    case UnaryOp::Plus:
        ss << "+";
        break;
    case UnaryOp::Minus:
        ss << "-";
        break;
    case UnaryOp::Not:
        ss << "!";
        break;
    }
    ss << "\n" << makeIndent(2) << unaryExp->toString();
    return ss.str();
}

std::string UnaryExp::toString() const {
    std::stringstream ss;
    ss << "UnaryExp\n" << makeIndent(2) << unaryExp->toString();
    return ss.str();
}

std::string FuncRParams::toString() const {
    std::stringstream ss;
    ss << "FuncRParams";
    if (!exps.empty()) {
        for (const auto &exp : exps) {
            ss << "\n" << makeIndent(2) << exp->toString();
        }
    }
    return ss.str();
}

std::string MulExp::MulExpOp::toString() const {
    std::stringstream ss;
    ss << "MulOp: ";
    switch (mulOp) {
    case MulOp::Mul:
        ss << "*";
        break;
    case MulOp::Div:
        ss << "/";
        break;
    case MulOp::Mod:
        ss << "%";
        break;
    }
    ss << "\n"
       << makeIndent(2) << "Left:\n"
       << makeIndent(4) << mulExp->toString();
    ss << "\n"
       << makeIndent(2) << "Right:\n"
       << makeIndent(4) << unaryExp->toString();
    return ss.str();
}

std::string MulExp::toString() const {
    std::stringstream ss;
    ss << "MulExp\n" << makeIndent(2) << mulExp->toString();
    return ss.str();
}

std::string AddExp::AddExpOp::toString() const {
    std::stringstream ss;
    ss << "AddOp: " << (addOp == AddOp::Plus ? "+" : "-");
    ss << "\n"
       << makeIndent(2) << "Left:\n"
       << makeIndent(4) << addExp->toString();
    ss << "\n"
       << makeIndent(2) << "Right:\n"
       << makeIndent(4) << mulExp->toString();
    return ss.str();
}

std::string AddExp::toString() const {
    std::stringstream ss;
    ss << "AddExp\n" << makeIndent(2) << addExp->toString();
    return ss.str();
}

std::string RelExp::RelExpOp::toString() const {
    std::stringstream ss;
    ss << "RelOp: ";
    switch (relOp) {
    case RelOp::Less:
        ss << "<";
        break;
    case RelOp::Greater:
        ss << ">";
        break;
    case RelOp::LessEqual:
        ss << "<=";
        break;
    case RelOp::GreaterEqual:
        ss << ">=";
        break;
    }
    ss << "\n"
       << makeIndent(2) << "Left:\n"
       << makeIndent(4) << relExp->toString();
    ss << "\n"
       << makeIndent(2) << "Right:\n"
       << makeIndent(4) << addExp->toString();
    return ss.str();
}

std::string RelExp::toString() const {
    std::stringstream ss;
    ss << "RelExp\n" << makeIndent(2) << relExp->toString();
    return ss.str();
}

std::string EqExp::EqExpOp::toString() const {
    std::stringstream ss;
    ss << "EqOp: " << (eqOp == EqOp::Equal ? "==" : "!=");
    ss << "\n"
       << makeIndent(2) << "Left:\n"
       << makeIndent(4) << eqExp->toString();
    ss << "\n"
       << makeIndent(2) << "Right:\n"
       << makeIndent(4) << relExp->toString();
    return ss.str();
}

std::string EqExp::toString() const {
    std::stringstream ss;
    ss << "EqExp\n" << makeIndent(2) << eqExp->toString();
    return ss.str();
}

std::string LAndExp::LAndExpOp::toString() const {
    std::stringstream ss;
    ss << "LogicalAnd";
    ss << "\n"
       << makeIndent(2) << "Left:\n"
       << makeIndent(4) << lAndExp->toString();
    ss << "\n"
       << makeIndent(2) << "Right:\n"
       << makeIndent(4) << eqExp->toString();
    return ss.str();
}

std::string LAndExp::toString() const {
    std::stringstream ss;
    ss << "LAndExp\n" << makeIndent(2) << lAndExp->toString();
    return ss.str();
}

std::string LOrExp::LOrExpOp::toString() const {
    std::stringstream ss;
    ss << "LogicalOr";
    ss << "\n"
       << makeIndent(2) << "Left:\n"
       << makeIndent(4) << lOrExp->toString();
    ss << "\n"
       << makeIndent(2) << "Right:\n"
       << makeIndent(4) << lAndExp->toString();
    return ss.str();
}

std::string LOrExp::toString() const {
    std::stringstream ss;
    ss << "LOrExp\n" << makeIndent(2) << lOrExp->toString();
    return ss.str();
}

std::string ConstExp::toString() const {
    std::stringstream ss;
    ss << "ConstExp\n" << makeIndent(2) << addExp->toString();
    return ss.str();
}

} // namespace ast
} // namespace frontend
