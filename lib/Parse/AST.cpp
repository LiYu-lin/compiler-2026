/**
 * @file AST.cpp
 * @author Dexer Matters (dexermatters@gmail.com)
 * @brief Implementation of the AST toString methods.
 * @version 0.3
 * @date 2025-07-12
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "AST.h"
#include <sstream>
#include "Visitor.h"
namespace frontend {
namespace ast {
IR::Value* ASTNode::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* CompUnit::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Decl::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* ConstDecl::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* ConstDef::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* ConstInitVal::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* VarDecl::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* VarDef::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* InitVal::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* FuncDef::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* FuncFParams::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* FuncFParam::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Block::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* BlockItem::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::AssignStmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::ExpStmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::BlockStmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::IfStmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::WhileStmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::BreakStmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::ContinueStmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Stmt::ReturnStmt::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Exp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* Cond::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* LVal::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* PrimaryExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* UnaryExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* UnaryExp::UnaryExpCall::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* UnaryExp::UnaryExpOp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* FuncRParams::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* MulExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* MulExp::MulExpOp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* AddExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* AddExp::AddExpOp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* RelExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* RelExp::RelExpOp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* EqExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* EqExp::EqExpOp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* LAndExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* LAndExp::LAndExpOp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* LOrExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* LOrExp::LOrExpOp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

IR::Value* ConstExp::accept(frontend::visitor::Visitor& visitor) const {
    return visitor.visit(*this);
}

std::string CompUnit::toString(int indent) const {
    std::stringstream ss;
    ss << "CompUnit";
    if (!decls.empty()) {
        for (const auto &decl : decls) {
            ss << "\n" << makeIndent(indent + 2) << decl->toString(indent + 2);
        }
    }
    return ss.str();
}

std::string Decl::toString(int indent) const {
    std::stringstream ss;
    ss << "Decl\n" << makeIndent(indent + 2) << decl->toString(indent + 2);
    return ss.str();
}

std::string ConstDecl::toString(int indent) const {
    std::stringstream ss;
    ss << "ConstDecl";
    ss << "\n"
       << makeIndent(indent + 2)
       << "BType: " << (btype == BType::Int ? "int" : "float");
    if (!constDefs.empty()) {
        for (const auto &def : constDefs) {
            ss << "\n" << makeIndent(indent + 2) << def->toString(indent + 2);
        }
    }
    return ss.str();
}

std::string ConstDef::toString(int indent) const {
    std::stringstream ss;
    ss << "ConstDef: " << ident;
    if (!dimensions.empty()) {
        ss << "\n" << makeIndent(indent + 2) << "Dimensions:";
        for (const auto &dim : dimensions) {
            ss << "\n" << makeIndent(indent + 4) << dim->toString(indent + 4);
        }
    }
    if (constInitVal) {
        ss << "\n"
           << makeIndent(indent + 2) << "InitValue:\n"
           << makeIndent(indent + 4) << constInitVal->toString(indent + 4);
    }
    return ss.str();
}

std::string ConstInitVal::toString(int indent) const {
    std::stringstream ss;
    ss << "ConstInitVal";
    if (std::holds_alternative<ASTNodePtr>(constInitVal)) {
        ss << "\n"
           << makeIndent(indent + 2)
           << std::get<ASTNodePtr>(constInitVal)->toString(indent + 2);
    } else if (std::holds_alternative<ASTNodePtrs>(constInitVal)) {
        const auto &vals = std::get<ASTNodePtrs>(constInitVal);
        if (!vals.empty()) {
            for (const auto &val : vals) {
                ss << "\n"
                   << makeIndent(indent + 2) << val->toString(indent + 2);
            }
        }
    }
    return ss.str();
}

std::string VarDecl::toString(int indent) const {
    std::stringstream ss;
    ss << "VarDecl";
    ss << "\n"
       << makeIndent(indent + 2)
       << "BType: " << (btype == BType::Int ? "int" : "float");
    if (!varDefs.empty()) {
        for (const auto &def : varDefs) {
            ss << "\n" << makeIndent(indent + 2) << def->toString(indent + 2);
        }
    }
    return ss.str();
}

std::string VarDef::toString(int indent) const {
    std::stringstream ss;
    ss << "VarDef: " << ident;
    if (!dimensions.empty()) {
        ss << "\n" << makeIndent(indent + 2) << "Dimensions:";
        for (const auto &dim : dimensions) {
            ss << "\n" << makeIndent(indent + 4) << dim->toString(indent + 4);
        }
    }
    if (initVal && *initVal) {
        ss << "\n"
           << makeIndent(indent + 2) << "InitValue:\n"
           << makeIndent(indent + 4) << (*initVal)->toString(indent + 4);
    }
    return ss.str();
}

std::string InitVal::toString(int indent) const {
    std::stringstream ss;
    ss << "InitVal";
    if (std::holds_alternative<ASTNodePtr>(initVal)) {
        ss << "\n"
           << makeIndent(indent + 2)
           << std::get<ASTNodePtr>(initVal)->toString(indent + 2);
    } else if (std::holds_alternative<ASTNodePtrs>(initVal)) {
        const auto &vals = std::get<ASTNodePtrs>(initVal);
        if (!vals.empty()) {
            for (const auto &val : vals) {
                ss << "\n"
                   << makeIndent(indent + 2) << val->toString(indent + 2);
            }
        }
    }
    return ss.str();
}

std::string FuncDef::toString(int indent) const {
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
       << makeIndent(indent + 2) << "Params:\n"
       << makeIndent(indent + 4) << funcFParams->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Body:\n"
       << makeIndent(indent + 4) << block->toString(indent + 4);
    return ss.str();
}

std::string FuncFParams::toString(int indent) const {
    std::stringstream ss;
    ss << "FuncFParams";
    if (!funcFParams.empty()) {
        for (const auto &param : funcFParams) {
            ss << "\n" << makeIndent(indent + 2) << param->toString(indent + 2);
        }
    }
    return ss.str();
}

std::string FuncFParam::toString(int indent) const {
    std::stringstream ss;
    ss << "FuncFParam: " << ident << " -> " << [&] {
        switch (btype) {
        case BType::Int:
            return "int";
        case BType::Float:
            return "float";
        }
        return "";
    }();
    if (dimensions && !dimensions->empty()) {
        ss << "\n" << makeIndent(indent + 2) << "Dimensions:";
        for (const auto &dim : *dimensions) {
            ss << "\n" << makeIndent(indent + 4) << dim->toString(indent + 4);
        }
    }
    return ss.str();
}

std::string Block::toString(int indent) const {
    std::stringstream ss;
    ss << "Block";
    if (!blockItems.empty()) {
        for (const auto &item : blockItems) {
            ss << "\n" << makeIndent(indent + 2) << item->toString(indent + 2);
        }
    }
    return ss.str();
}

std::string BlockItem::toString(int indent) const {
    std::stringstream ss;
    ss << "BlockItem\n" << makeIndent(indent + 2) << item->toString(indent + 2);
    return ss.str();
}

std::string Stmt::AssignStmt::toString(int indent) const {
    std::stringstream ss;
    ss << "AssignStmt";
    ss << "\n"
       << makeIndent(indent + 2) << "LVal:\n"
       << makeIndent(indent + 4) << lval->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Expr:\n"
       << makeIndent(indent + 4) << exp->toString(indent + 4);
    return ss.str();
}

std::string Stmt::ExpStmt::toString(int indent) const {
    std::stringstream ss;
    ss << "ExpStmt";
    if (exp && *exp) {
        ss << "\n" << makeIndent(indent + 2) << (*exp)->toString(indent + 2);
    }
    return ss.str();
}

std::string Stmt::BlockStmt::toString(int indent) const {
    std::stringstream ss;
    ss << "BlockStmt\n"
       << makeIndent(indent + 2) << block->toString(indent + 2);
    return ss.str();
}

std::string Stmt::IfStmt::toString(int indent) const {
    std::stringstream ss;
    ss << "IfStmt";
    ss << "\n"
       << makeIndent(indent + 2) << "Condition:\n"
       << makeIndent(indent + 4) << exp->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Then:\n"
       << makeIndent(indent + 4) << block->toString(indent + 4);
    if (elseStmt && *elseStmt) {
        ss << "\n"
           << makeIndent(indent + 2) << "Else:\n"
           << makeIndent(indent + 4) << (*elseStmt)->toString(indent + 4);
    }
    return ss.str();
}

std::string Stmt::WhileStmt::toString(int indent) const {
    std::stringstream ss;
    ss << "WhileStmt";
    ss << "\n"
       << makeIndent(indent + 2) << "Condition:\n"
       << makeIndent(indent + 4) << cond->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Body:\n"
       << makeIndent(indent + 4) << stmt->toString(indent + 4);
    return ss.str();
}

std::string Stmt::BreakStmt::toString(int _indent) const { return "BreakStmt"; }

std::string Stmt::ContinueStmt::toString(int _indent) const {
    return "ContinueStmt";
}

std::string Stmt::ReturnStmt::toString(int indent) const {
    std::stringstream ss;
    ss << "ReturnStmt";
    if (exp && *exp) {
        ss << "\n" << makeIndent(indent + 2) << (*exp)->toString(indent + 2);
    }
    return ss.str();
}

std::string Stmt::toString(int indent) const {
    std::stringstream ss;
    ss << "Stmt\n" << makeIndent(indent + 2) << stmt->toString(indent + 2);
    return ss.str();
}

std::string LVal::toString(int indent) const {
    std::stringstream ss;
    ss << "LVal: " << ident;
    if (!dimensions.empty()) {
        ss << "\n" << makeIndent(indent + 2) << "Dimensions:";
        for (const auto &dim : dimensions) {
            ss << "\n" << makeIndent(indent + 4) << dim->toString(indent + 4);
        }
    }
    return ss.str();
}

std::string PrimaryExp::toString(int indent) const {
    std::stringstream ss;
    ss << "PrimaryExp";
    if (std::holds_alternative<ASTNodePtr>(primaryExp)) {
        ss << "\n"
           << makeIndent(indent + 2)
           << std::get<ASTNodePtr>(primaryExp)->toString(indent + 2);
    } else if (std::holds_alternative<Number>(primaryExp)) {
        ss << ": " << std::get<Number>(primaryExp).toString();
    }
    return ss.str();
}

std::string UnaryExp::UnaryExpCall::toString(int indent) const {
    std::stringstream ss;
    ss << "FunctionCall: " << ident;
    if (!funcRParams.empty()) {
        ss << "\n" << makeIndent(indent + 2) << "Args:";
        for (const auto &param : funcRParams) {
            ss << "\n" << makeIndent(indent + 4) << param->toString(indent + 4);
        }
    }
    return ss.str();
}

std::string UnaryExp::UnaryExpOp::toString(int indent) const {
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
    ss << "\n" << makeIndent(indent + 2) << unaryExp->toString(indent + 2);
    return ss.str();
}

std::string UnaryExp::toString(int indent) const {
    std::stringstream ss;
    ss << "UnaryExp\n"
       << makeIndent(indent + 2) << unaryExp->toString(indent + 2);
    return ss.str();
}

std::string FuncRParams::toString(int indent) const {
    std::stringstream ss;
    ss << "FuncRParams";
    if (!exps.empty()) {
        for (const auto &exp : exps) {
            ss << "\n" << makeIndent(indent + 2) << exp->toString(indent + 2);
        }
    }
    return ss.str();
}

std::string MulExp::MulExpOp::toString(int indent) const {
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
       << makeIndent(indent + 2) << "Left:\n"
       << makeIndent(indent + 4) << mulExp->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Right:\n"
       << makeIndent(indent + 4) << unaryExp->toString(indent + 4);
    return ss.str();
}

std::string MulExp::toString(int indent) const {
    std::stringstream ss;
    ss << "MulExp\n" << makeIndent(indent + 2) << mulExp->toString(indent + 2);
    return ss.str();
}

std::string AddExp::AddExpOp::toString(int indent) const {
    std::stringstream ss;
    ss << "AddOp: " << (addOp == AddOp::Plus ? "+" : "-");
    ss << "\n"
       << makeIndent(indent + 2) << "Left:\n"
       << makeIndent(indent + 4) << addExp->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Right:\n"
       << makeIndent(indent + 4) << mulExp->toString(indent + 4);
    return ss.str();
}

std::string AddExp::toString(int indent) const {
    std::stringstream ss;
    ss << "AddExp\n" << makeIndent(indent + 2) << addExp->toString(indent + 2);
    return ss.str();
}

std::string RelExp::RelExpOp::toString(int indent) const {
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
       << makeIndent(indent + 2) << "Left:\n"
       << makeIndent(indent + 4) << relExp->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Right:\n"
       << makeIndent(indent + 4) << addExp->toString(indent + 4);
    return ss.str();
}

std::string RelExp::toString(int indent) const {
    std::stringstream ss;
    ss << "RelExp\n" << makeIndent(indent + 2) << relExp->toString(indent + 2);
    return ss.str();
}

std::string EqExp::EqExpOp::toString(int indent) const {
    std::stringstream ss;
    ss << "EqOp: " << (eqOp == EqOp::Equal ? "==" : "!=");
    ss << "\n"
       << makeIndent(indent + 2) << "Left:\n"
       << makeIndent(indent + 4) << eqExp->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Right:\n"
       << makeIndent(indent + 4) << relExp->toString(indent + 4);
    return ss.str();
}

std::string EqExp::toString(int indent) const {
    std::stringstream ss;
    ss << "EqExp\n" << makeIndent(indent + 2) << eqExp->toString(indent + 2);
    return ss.str();
}

std::string LAndExp::LAndExpOp::toString(int indent) const {
    std::stringstream ss;
    ss << "LogicalAnd";
    ss << "\n"
       << makeIndent(indent + 2) << "Left:\n"
       << makeIndent(indent + 4) << lAndExp->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Right:\n"
       << makeIndent(indent + 4) << eqExp->toString(indent + 4);
    return ss.str();
}

std::string LAndExp::toString(int indent) const {
    std::stringstream ss;
    ss << "LAndExp\n"
       << makeIndent(indent + 2) << lAndExp->toString(indent + 2);
    return ss.str();
}

std::string LOrExp::LOrExpOp::toString(int indent) const {
    std::stringstream ss;
    ss << "LogicalOr";
    ss << "\n"
       << makeIndent(indent + 2) << "Left:\n"
       << makeIndent(indent + 4) << lOrExp->toString(indent + 4);
    ss << "\n"
       << makeIndent(indent + 2) << "Right:\n"
       << makeIndent(indent + 4) << lAndExp->toString(indent + 4);
    return ss.str();
}

std::string LOrExp::toString(int indent) const {
    std::stringstream ss;
    ss << "LOrExp\n" << makeIndent(indent + 2) << lOrExp->toString(indent + 2);
    return ss.str();
}

std::string ConstExp::toString(int indent) const {
    std::stringstream ss;
    ss << "ConstExp\n"
       << makeIndent(indent + 2) << addExp->toString(indent + 2);
    return ss.str();
}

std::string Exp::toString(int indent) const {
    std::stringstream ss;
    ss << "Exp\n" << makeIndent(indent + 2) << addExp->toString(indent + 2);
    return ss.str();
}

std::string Cond::toString(int indent) const {
    std::stringstream ss;
    ss << "Cond\n" << makeIndent(indent + 2) << lOrExp->toString(indent + 2);
    return ss.str();
}

} // namespace ast
} // namespace frontend
