// /**
//  * @file SymbolBuild.h
//  * @author CoffeeRain
//  * @brief Header file for the SymbolBuilder class.
//  * @version 0.1
//  * @date 2025-05-29
//  *
//  * @copyright Copyright (c) 2025
//  *
//  */
// #pragma once
// #include "Parse/AST.h"
// #include "Visitor.h"
// #include <ostream>

// namespace frontend {
//     namespace visitor {
//     class SymbolBuilder {
//         SymbolTable& symTable;
//         BType currentBType;
//         bool isConstDecl = false;

//     public:
//         SymbolBuilder(SymbolTable& table) : symTable(table) {}

//         void build(const ast::ASTNodePtr& node) {
//             if (!node) return;
//             using Type = ast::ASTNode::Type;
//             switch (node->type) {
//             case Type::CompUnit:
//                 handleCompUnit(static_cast<ast::CompUnit&>(*node));
//                 break;
//             case Type::Decl:
//                 handleDecl(static_cast<ast::Decl&>(*node));
//                 break;
//             case Type::ConstDecl:
//                 handleConstDecl(static_cast<ast::ConstDecl&>(*node));
//                 break;
//             case Type::ConstDef:
//                 handleConstDef(static_cast<ast::ConstDef&>(*node));
//                 break;
//             case Type::VarDecl:
//                 handleVarDecl(static_cast<ast::VarDecl&>(*node));
//                 break;
//             case Type::VarDef:
//                 handleVarDef(static_cast<ast::VarDef&>(*node));
//                 break;
//             case Type::FuncDef:
//                 handleFuncDef(static_cast<ast::FuncDef&>(*node));
//                 break;
//             case Type::FuncFParams:
//                 handleFuncFParams(static_cast<ast::FuncFParams&>(*node));
//                 break;
//             case Type::FuncFParam:
//                 handleFuncFParam(static_cast<ast::FuncFParam&>(*node));
//                 break;
//             case Type::Block:
//                 handleBlock(static_cast<ast::Block&>(*node));
//                 break;
//             case Type::BlockItem:
//                 handleBlockItem(static_cast<ast::BlockItem&>(*node));
//                 break;
//             case Type::Stmt: {
//                 auto& stmt = static_cast<ast::Stmt&>(*node);
//                 if (stmt.stmt) build(stmt.stmt);
//                 break;
//             }
//             case Type::AssignStmt: {
//                 auto& assign = static_cast<ast::Stmt::AssignStmt&>(*node);
//                 build(assign.lval);
//                 build(assign.exp);
//                 break;
//             }
//             case Type::ExpStmt: {
//                 auto& expStmt = static_cast<ast::Stmt::ExpStmt&>(*node);
//                 if (expStmt.exp) build(*expStmt.exp);
//                 break;
//             }
//             case Type::BlockStmt: {
//                 auto& blockStmt = static_cast<ast::Stmt::BlockStmt&>(*node);
//                 build(blockStmt.block);
//                 break;
//             }
//             case Type::IfStmt: {
//                 auto& ifStmt = static_cast<ast::Stmt::IfStmt&>(*node);
//                 build(ifStmt.exp);
//                 build(ifStmt.block);
//                 if (ifStmt.elseStmt) build(*ifStmt.elseStmt);
//                 break;
//             }
//             case Type::WhileStmt: {
//                 auto& whileStmt = static_cast<ast::Stmt::WhileStmt&>(*node);
//                 build(whileStmt.cond);
//                 build(whileStmt.stmt);
//                 break;
//             }
//             case Type::BreakStmt:
//             case Type::ContinueStmt:
//                 break;
//             case Type::ReturnStmt: {
//                 auto& ret = static_cast<ast::Stmt::ReturnStmt&>(*node);
//                 if (ret.exp) build(*ret.exp);
//                 break;
//             }
//             case Type::Exp: {
//                 auto& exp = static_cast<ast::Exp&>(*node);
//                 build(exp.addExp);
//                 break;
//             }
//             case Type::Cond: {
//                 auto& cond = static_cast<ast::Cond&>(*node);
//                 build(cond.lOrExp);
//                 break;
//             }
//             case Type::LVal: {
//                 auto& lval = static_cast<ast::LVal&>(*node);
//                 for (auto& dim : lval.dimensions) build(dim);
//                 break;
//             }
//             case Type::PrimaryExp: {
//                 auto& prim = static_cast<ast::PrimaryExp&>(*node);
//                 if (std::holds_alternative<ast::ASTNodePtr>(prim.primaryExp)) {
//                     build(std::get<ast::ASTNodePtr>(prim.primaryExp));
//                 }
//                 break;
//             }
//             case Type::UnaryExp: {
//                 auto& unary = static_cast<ast::UnaryExp&>(*node);
//                 build(unary.unaryExp);
//                 break;
//             }
//             case Type::UnaryExpCall: {
//                 auto& call = static_cast<ast::UnaryExp::UnaryExpCall&>(*node);
//                 for (auto& param : call.funcRParams) build(param);
//                 break;
//             }
//             case Type::UnaryExpOp: {
//                 auto& op = static_cast<ast::UnaryExp::UnaryExpOp&>(*node);
//                 build(op.unaryExp);
//                 break;
//             }
//             case Type::FuncRParams: {
//                 auto& params = static_cast<ast::FuncRParams&>(*node);
//                 for (auto& exp : params.exps) build(exp);
//                 break;
//             }
//             case Type::MulExp: {
//                 auto& mul = static_cast<ast::MulExp&>(*node);
//                 build(mul.mulExp);
//                 break;
//             }
//             case Type::MulExpOp: {
//                 auto& op = static_cast<ast::MulExp::MulExpOp&>(*node);
//                 build(op.mulExp);
//                 build(op.unaryExp);
//                 break;
//             }
//             case Type::AddExp: {
//                 auto& add = static_cast<ast::AddExp&>(*node);
//                 build(add.addExp);
//                 break;
//             }
//             case Type::AddExpOp: {
//                 auto& op = static_cast<ast::AddExp::AddExpOp&>(*node);
//                 build(op.addExp);
//                 build(op.mulExp);
//                 break;
//             }
//             case Type::RelExp: {
//                 auto& rel = static_cast<ast::RelExp&>(*node);
//                 build(rel.relExp);
//                 break;
//             }
//             case Type::RelExpOp: {
//                 auto& op = static_cast<ast::RelExp::RelExpOp&>(*node);
//                 build(op.relExp);
//                 build(op.addExp);
//                 break;
//             }
//             case Type::EqExp: {
//                 auto& eq = static_cast<ast::EqExp&>(*node);
//                 build(eq.eqExp);
//                 break;
//             }
//             case Type::EqExpOp: {
//                 auto& op = static_cast<ast::EqExp::EqExpOp&>(*node);
//                 build(op.eqExp);
//                 build(op.relExp);
//                 break;
//             }
//             case Type::LAndExp: {
//                 auto& land = static_cast<ast::LAndExp&>(*node);
//                 build(land.lAndExp);
//                 break;
//             }
//             case Type::LAndExpOp: {
//                 auto& op = static_cast<ast::LAndExp::LAndExpOp&>(*node);
//                 build(op.lAndExp);
//                 build(op.eqExp);
//                 break;
//             }
//             case Type::LOrExp: {
//                 auto& lor = static_cast<ast::LOrExp&>(*node);
//                 build(lor.lOrExp);
//                 break;
//             }
//             case Type::LOrExpOp: {
//                 auto& op = static_cast<ast::LOrExp::LOrExpOp&>(*node);
//                 build(op.lOrExp);
//                 build(op.lAndExp);
//                 break;
//             }
//             case Type::ConstExp: {
//                 auto& cexp = static_cast<ast::ConstExp&>(*node);
//                 build(cexp.addExp);
//                 break;
//             }
//             case Type::ConstInitVal: {
//                 auto& civ = static_cast<ast::ConstInitVal&>(*node);
//                 if (std::holds_alternative<ast::ASTNodePtrs>(civ.constInitVal)) {
//                     for (auto& n : std::get<ast::ASTNodePtrs>(civ.constInitVal)) build(n);
//                 } else {
//                     build(std::get<ast::ASTNodePtr>(civ.constInitVal));
//                 }
//                 break;
//             }
//             case Type::InitVal: {
//                 auto& iv = static_cast<ast::InitVal&>(*node);
//                 if (std::holds_alternative<ast::ASTNodePtrs>(iv.initVal)) {
//                     for (auto& n : std::get<ast::ASTNodePtrs>(iv.initVal)) build(n);
//                 } else {
//                     build(std::get<ast::ASTNodePtr>(iv.initVal));
//                 }
//                 break;
//             }
//             default:
//                 break;
//             }
//         }

//     private:
//         void handleCompUnit(const ast::CompUnit& compUnit) {
//             std::cout<<"compunit.num"<<compUnit.decls.size()<<std::endl;
//             for (const auto& decl : compUnit.decls) {
//                 build(decl);
//             }
//         }

//         void handleDecl(const ast::Decl& decl) {
//             build(decl.decl);
//         }

//         void handleConstDecl(const ast::ConstDecl& constDecl) {
//             isConstDecl = true;
//             currentBType = BType(constDecl.btype);
//             for (const auto& def : constDecl.constDefs) {
//                 build(def);
//             }
//             isConstDecl = false;
//         }

//         void handleConstDef(const ast::ConstDef& constDef) {
//             std::vector<int> dims;
//             for (const auto& dim : constDef.dimensions) {
//                 dims.push_back(0);
//             }

//             SymbolInfo info(
//                 constDef.ident,
//                 currentBType,
//                 true, 
//                 symTable.getCurrentScopeLevel()
//             );
//             info.dims = dims;

//             if (!symTable.insert(constDef.ident, info)) {
//                 throw ("重复定义常量: " + constDef.ident);
//             }
//         }

//         void handleFuncDef(const ast::FuncDef& funcDef) {
//             SymbolInfo funcInfo(
//                 funcDef.ident,
//                 FuncType(funcDef.funcType),
//                 symTable.getCurrentScopeLevel()
//             );
//             if (!symTable.insert(funcDef.ident, funcInfo)) {
//                 throw ("重复定义函数: " + funcDef.ident);
//             }

//             if(funcDef.funcFParams) {
//                 build(funcDef.funcFParams);
//             }

//             build(funcDef.block);
//         }
//         void handleFuncFParams(const ast::FuncFParams& funcFParams) {
//             for (const auto& param : funcFParams.funcFParams) {
//                 build(param);
//             }
//         }
//         void handleFuncFParam(const ast::FuncFParam& funcFParam) {
//             SymbolInfo info(
//                 funcFParam.ident, 
//                 BType(funcFParam.btype), 
//                 false, 
//                 symTable.getCurrentScopeLevel()
//             );

//             symTable.insert(info.name, info);
//         }

//     void handleVarDecl(const ast::VarDecl& varDecl) {
//         currentBType = BType(varDecl.btype);
//         for (const auto& varDef : varDecl.varDefs) {
//             build(varDef);
//         }
//     }

//     void handleVarDef(const ast::VarDef& varDef) {
//         std::vector<int> dims;
//         for (const auto& dim : varDef.dimensions) {
//             dims.push_back(0); 
//         }

//         SymbolInfo info(
//             varDef.ident,
//             currentBType,
//             false,
//             symTable.getCurrentScopeLevel()
//         );
//         info.dims = dims;
//         if (!symTable.insert(varDef.ident, info)) {
//             throw ("重复定义变量: " + varDef.ident);
//         }
//     }

//     void handleBlock(const ast::Block& block) {
//         symTable.enterScope();
//         for (const auto& item : block.blockItems) {
//             build(item);
//         }
//         symTable.exitScope();
//     }

//     void handleBlockItem(const ast::BlockItem& blockItem) {
//         if (blockItem.item) {
//             build(blockItem.item);
//         }

//     };




//     };

//     } 
// }