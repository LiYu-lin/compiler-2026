/**
 * @file AST.h
 * @author Dexer Matters (dexermatters@gmail.com)
 * @brief A file containing the definition of the AST class.
 * @version 0.1
 * @date 2025-05-11
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include "Value.h"
namespace frontend::visitor {
    class Visitor;
}
namespace frontend {
namespace ast {

struct ASTNode {
    enum class BType {
        Int,
        Float,
    };
    enum class FuncType {
        Void,
        Int,
        Float,
    };
    enum class UnaryOp {
        Plus,
        Minus,
        Not,
    };
    enum class MulOp {
        Mul,
        Div,
        Mod,
    };
    enum class AddOp {
        Plus,
        Minus,
    };
    enum class RelOp {
        Less,
        Greater,
        LessEqual,
        GreaterEqual,
    };
    enum class EqOp {
        Equal,
        NotEqual,
    };
    enum class Type {
        CompUnit,
        Decl,
        ConstDecl,
        BType,
        ConstDef,
        ConstInitVal,
        VarDecl,
        VarDef,
        InitVal,
        FuncDef,
        FuncType,
        FuncFParams,
        FuncFParam,
        Block,
        BlockItem,
        Stmt,
        AssignStmt,
        ExpStmt,
        BlockStmt,
        IfStmt,
        WhileStmt,
        BreakStmt,
        ContinueStmt,
        ReturnStmt,
        Exp,
        LVal,
        PrimaryExp,
        Number,
        UnaryExp,
        UnaryOp,
        UnaryExpCall,
        UnaryExpOp,
        FuncRParams,
        MulExp,
        MulExpOp,
        AddExp,
        AddExpOp,
        RelExp,
        RelExpOp,
        EqExp,
        EqExpOp,
        LAndExp,
        LAndExpOp,
        LOrExp,
        LOrExpOp,
        ConstExp,
        Cond,
    };
    Type type;

    ASTNode() = default;
    ASTNode(Type type) : type(type) {}
    virtual ~ASTNode() = default;
    virtual std::string toString(int indent) const = 0;

    std::string makeIndent(int indent) const {
        return std::string(indent, ' ');
    }
    virtual IR::Value* accept(frontend::visitor::Visitor& visitor) const = 0;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;
using ASTNodePtrs = std::vector<ASTNodePtr>;
using OptASTNodePtr = std::optional<ASTNodePtr>;
using OptASTNodePtrs = std::optional<ASTNodePtrs>;

struct CompUnit : public ASTNode {
    ASTNodePtrs decls;
    CompUnit() : ASTNode(Type::CompUnit) {}
    CompUnit(ASTNodePtrs decls)
        : ASTNode(Type::CompUnit), decls(std::move(decls)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct Decl : public ASTNode {
    ASTNodePtr decl; // ConstDecl or VarDecl
    Decl() : ASTNode(Type::Decl) {}
    Decl(ASTNodePtr decl) : ASTNode(Type::Decl), decl(std::move(decl)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct ConstDecl : public ASTNode {
    BType btype;
    ASTNodePtrs constDefs;
    ConstDecl() : ASTNode(Type::ConstDecl) {}
    ConstDecl(BType btype, ASTNodePtrs constDefs)
        : ASTNode(Type::ConstDecl), btype(std::move(btype)),
          constDefs(std::move(constDefs)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct ConstDef : public ASTNode {
    std::string ident;
    ASTNodePtrs dimensions;
    ASTNodePtr constInitVal;
    ConstDef() : ASTNode(Type::ConstDef) {}
    ConstDef(std::string ident, ASTNodePtrs dimensions, ASTNodePtr constInitVal)
        : ASTNode(Type::ConstDef), ident(std::move(ident)),
          dimensions(std::move(dimensions)),
          constInitVal(std::move(constInitVal)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct ConstInitVal : public ASTNode {
    std::variant<ASTNodePtrs, ASTNodePtr> constInitVal;
    ConstInitVal() : ASTNode(Type::ConstInitVal) {}
    ConstInitVal(ASTNodePtrs constInitVal)
        : ASTNode(Type::ConstInitVal), constInitVal(std::move(constInitVal)) {}
    ConstInitVal(ASTNodePtr constInitVal)
        : ASTNode(Type::ConstInitVal), constInitVal(std::move(constInitVal)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct VarDecl : public ASTNode {
    BType btype;
    ASTNodePtrs varDefs;
    VarDecl() : ASTNode(Type::VarDecl) {}
    VarDecl(BType btype, ASTNodePtrs varDefs)
        : ASTNode(Type::VarDecl), btype(std::move(btype)),
          varDefs(std::move(varDefs)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct VarDef : public ASTNode {
    std::string ident;
    ASTNodePtrs dimensions;
    OptASTNodePtr initVal;
    VarDef() : ASTNode(Type::VarDef) {}
    VarDef(std::string ident, ASTNodePtrs dimensions,
           OptASTNodePtr initVal = std::nullopt)
        : ASTNode(Type::VarDef), ident(std::move(ident)),
          dimensions(std::move(dimensions)), initVal(std::move(initVal)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct InitVal : public ASTNode {
    std::variant<ASTNodePtrs, ASTNodePtr> initVal;
    InitVal() : ASTNode(Type::InitVal) {}
    InitVal(ASTNodePtrs initVal)
        : ASTNode(Type::InitVal), initVal(std::move(initVal)) {}
    InitVal(ASTNodePtr initVal)
        : ASTNode(Type::InitVal), initVal(std::move(initVal)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct FuncDef : public ASTNode {
    FuncType funcType;
    std::string ident;
    ASTNodePtr funcFParams;
    ASTNodePtr block;
    FuncDef() : ASTNode(Type::FuncDef) {}
    FuncDef(FuncType funcType, std::string ident, ASTNodePtr funcFParams,
            ASTNodePtr block)
        : ASTNode(Type::FuncDef), funcType(std::move(funcType)),
          ident(std::move(ident)), funcFParams(std::move(funcFParams)),
          block(std::move(block)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct FuncFParams : public ASTNode {
    ASTNodePtrs funcFParams;
    FuncFParams() : ASTNode(Type::FuncFParams) {}
    FuncFParams(ASTNodePtrs funcFParams)
        : ASTNode(Type::FuncFParams), funcFParams(std::move(funcFParams)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct FuncFParam : public ASTNode {
    BType btype;
    std::string ident;
    OptASTNodePtrs dimensions;
    FuncFParam() : ASTNode(Type::FuncFParam) {}
    FuncFParam(BType btype, std::string ident,
               OptASTNodePtrs dimensions = std::nullopt)
        : ASTNode(Type::FuncFParam), btype(std::move(btype)),
          ident(std::move(ident)), dimensions(std::move(dimensions)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct Block : public ASTNode {
    ASTNodePtrs blockItems;
    Block() : ASTNode(Type::Block) {}
    Block(ASTNodePtrs blockItems)
        : ASTNode(Type::Block), blockItems(std::move(blockItems)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct BlockItem : public ASTNode {
    ASTNodePtr item; // Stmt or Decl
    BlockItem() : ASTNode(Type::BlockItem) {}
    BlockItem(ASTNodePtr item)
        : ASTNode(Type::BlockItem), item(std::move(item)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct Stmt : public ASTNode {
    struct AssignStmt : public ASTNode {
        ASTNodePtr lval;
        ASTNodePtr exp;
        AssignStmt() : ASTNode(Type::AssignStmt) {}
        AssignStmt(ASTNodePtr lval, ASTNodePtr exp)
            : ASTNode(Type::AssignStmt), lval(std::move(lval)),
              exp(std::move(exp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    struct ExpStmt : public ASTNode {
        OptASTNodePtr exp;
        ExpStmt(OptASTNodePtr exp = std::nullopt)
            : ASTNode(Type::ExpStmt), exp(std::move(exp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    struct BlockStmt : public ASTNode {
        ASTNodePtr block;
        BlockStmt(ASTNodePtr block)
            : ASTNode(Type::BlockStmt), block(std::move(block)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    struct IfStmt : public ASTNode {
        ASTNodePtr exp;
        ASTNodePtr block;
        OptASTNodePtr elseStmt;
        IfStmt(ASTNodePtr exp, ASTNodePtr block,
               OptASTNodePtr elseStmt = std::nullopt)
            : ASTNode(Type::IfStmt), exp(std::move(exp)),
              block(std::move(block)), elseStmt(std::move(elseStmt)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    struct WhileStmt : public ASTNode {
        ASTNodePtr cond;
        ASTNodePtr stmt;
        WhileStmt(ASTNodePtr cond, ASTNodePtr stmt)
            : ASTNode(Type::WhileStmt), cond(std::move(cond)),
              stmt(std::move(stmt)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    struct BreakStmt : public ASTNode {
        BreakStmt() : ASTNode(Type::BreakStmt) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    struct ContinueStmt : public ASTNode {
        ContinueStmt() : ASTNode(Type::ContinueStmt) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    struct ReturnStmt : public ASTNode {
        OptASTNodePtr exp;
        ReturnStmt(OptASTNodePtr exp = std::nullopt)
            : ASTNode(Type::ReturnStmt), exp(std::move(exp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    ASTNodePtr stmt;
    Stmt() : ASTNode(Type::Stmt) {}
    Stmt(ASTNodePtr stmt) : ASTNode(Type::Stmt), stmt(std::move(stmt)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct Exp : public ASTNode {
    ASTNodePtr addExp;
    Exp() : ASTNode(Type::Exp) {}
    Exp(ASTNodePtr addExp) : ASTNode(Type::Exp), addExp(std::move(addExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct Cond : public ASTNode {
    ASTNodePtr lOrExp;
    Cond(ASTNodePtr lOrExp) : ASTNode(Type::Cond), lOrExp(std::move(lOrExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct LVal : public ASTNode {
    std::string ident;
    ASTNodePtrs dimensions;
    LVal() : ASTNode(Type::LVal) {}
    LVal(std::string ident, ASTNodePtrs dimensions)
        : ASTNode(Type::LVal), ident(std::move(ident)),
          dimensions(std::move(dimensions)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct Number {
    std::variant<int, float> value;
    Number(int value) : value(value) {}
    Number(float value) : value(value) {}
    Number(std::variant<int, float> value) : value(std::move(value)) {}
    std::string toString() const {
        if (std::holds_alternative<int>(value)) {
            return std::to_string(std::get<int>(value));
        } else {
            return std::to_string(std::get<float>(value));
        }
    }
    IR::Value* accept(frontend::visitor::Visitor& visitor) const;
};

struct PrimaryExp : public ASTNode {
    std::variant<ASTNodePtr, Number> primaryExp;
    PrimaryExp(ASTNodePtr primaryExp)
        : ASTNode(Type::PrimaryExp), primaryExp(std::move(primaryExp)) {}
    PrimaryExp(Number number)
        : ASTNode(Type::PrimaryExp), primaryExp(std::move(number)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct UnaryExp : public ASTNode {
    struct UnaryExpCall : public ASTNode {
        std::string ident;
        ASTNodePtrs funcRParams;
        UnaryExpCall(std::string ident, ASTNodePtrs funcRParams)
            : ASTNode(Type::UnaryExpCall), ident(std::move(ident)),
              funcRParams(std::move(funcRParams)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    struct UnaryExpOp : public ASTNode {
        UnaryOp unaryOp;
        ASTNodePtr unaryExp;
        UnaryExpOp(UnaryOp unaryOp, ASTNodePtr unaryExp)
            : ASTNode(Type::UnaryExpOp), unaryOp(std::move(unaryOp)),
              unaryExp(std::move(unaryExp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    ASTNodePtr unaryExp;
    UnaryExp(ASTNodePtr unaryExp)
        : ASTNode(Type::UnaryExp), unaryExp(std::move(unaryExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct FuncRParams : public ASTNode {
    ASTNodePtrs exps;
    FuncRParams() : ASTNode(Type::FuncRParams) {}
    FuncRParams(ASTNodePtrs exps)
        : ASTNode(Type::FuncRParams), exps(std::move(exps)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct MulExp : public ASTNode {
    struct MulExpOp : public ASTNode {
        MulOp mulOp;
        ASTNodePtr mulExp;
        ASTNodePtr unaryExp;
        MulExpOp(MulOp mulOp, ASTNodePtr mulExp, ASTNodePtr unaryExp)
            : ASTNode(Type::MulExpOp), mulOp(std::move(mulOp)),
              mulExp(std::move(mulExp)), unaryExp(std::move(unaryExp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    ASTNodePtr mulExp; // MulExpOp or UnaryExp
    MulExp() : ASTNode(Type::MulExp) {}
    MulExp(ASTNodePtr mulExp)
        : ASTNode(Type::MulExp), mulExp(std::move(mulExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;

};

struct AddExp : public ASTNode {
    struct AddExpOp : public ASTNode {
        AddOp addOp;
        ASTNodePtr addExp;
        ASTNodePtr mulExp;
        AddExpOp(AddOp addOp, ASTNodePtr addExp, ASTNodePtr mulExp)
            : ASTNode(Type::AddExpOp), addOp(std::move(addOp)),
              addExp(std::move(addExp)), mulExp(std::move(mulExp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    ASTNodePtr addExp; // AddExpOp or MulExp
    AddExp() : ASTNode(Type::AddExp) {}
    AddExp(ASTNodePtr addExp)
        : ASTNode(Type::AddExp), addExp(std::move(addExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;

};

struct RelExp : public ASTNode {
    struct RelExpOp : public ASTNode {
        RelOp relOp;
        ASTNodePtr relExp;
        ASTNodePtr addExp;
        RelExpOp(RelOp relOp, ASTNodePtr relExp, ASTNodePtr addExp)
            : ASTNode(Type::RelExpOp), relOp(std::move(relOp)),
              relExp(std::move(relExp)), addExp(std::move(addExp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    ASTNodePtr relExp; // RelExpOp or AddExp
    RelExp() : ASTNode(Type::RelExp) {}
    RelExp(ASTNodePtr relExp)
        : ASTNode(Type::RelExp), relExp(std::move(relExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;

};

struct EqExp : public ASTNode {
    struct EqExpOp : public ASTNode {
        EqOp eqOp;
        ASTNodePtr eqExp;
        ASTNodePtr relExp;
        EqExpOp(EqOp eqOp, ASTNodePtr eqExp, ASTNodePtr relExp)
            : ASTNode(Type::EqExpOp), eqOp(std::move(eqOp)),
              eqExp(std::move(eqExp)), relExp(std::move(relExp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    ASTNodePtr eqExp; // EqExpOp or RelExp
    EqExp() : ASTNode(Type::EqExp) {}
    EqExp(ASTNodePtr eqExp) : ASTNode(Type::EqExp), eqExp(std::move(eqExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct LAndExp : public ASTNode {
    struct LAndExpOp : public ASTNode {
        ASTNodePtr lAndExp;
        ASTNodePtr eqExp;
        LAndExpOp(ASTNodePtr lAndExp, ASTNodePtr eqExp)
            : ASTNode(Type::LAndExpOp), lAndExp(std::move(lAndExp)),
              eqExp(std::move(eqExp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    ASTNodePtr lAndExp; // LAndExpOp or EqExp
    LAndExp() : ASTNode(Type::LAndExp) {}
    LAndExp(ASTNodePtr lAndExp)
        : ASTNode(Type::LAndExp), lAndExp(std::move(lAndExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct LOrExp : public ASTNode {
    struct LOrExpOp : public ASTNode {
        ASTNodePtr lOrExp;
        ASTNodePtr lAndExp;
        LOrExpOp(ASTNodePtr lOrExp, ASTNodePtr lAndExp)
            : ASTNode(Type::LOrExpOp), lOrExp(std::move(lOrExp)),
              lAndExp(std::move(lAndExp)) {}
        std::string toString(int indent) const override;
        IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
    };

    ASTNodePtr lOrExp; // LOrExpOp or LAndExp
    LOrExp() : ASTNode(Type::LOrExp) {}
    LOrExp(ASTNodePtr lOrExp)
        : ASTNode(Type::LOrExp), lOrExp(std::move(lOrExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;
};

struct ConstExp : public ASTNode {
    ASTNodePtr addExp;
    ConstExp(ASTNodePtr addExp)
        : ASTNode(Type::ConstExp), addExp(std::move(addExp)) {}
    std::string toString(int indent) const override;
    IR::Value* accept(frontend::visitor::Visitor& visitor) const override;

};

} // namespace ast
} // namespace frontend