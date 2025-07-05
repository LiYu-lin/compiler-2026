// IRBuilder.cpp
#include "IR/IR_Builder.h"
#include "IR/IR_Value.h"
#include "Parse/AST.h"

namespace IR {

std::unique_ptr<IRModule> IRBuilder::build(const frontend::ast::ASTNodePtr& node) {
    if (!node) return nullptr;
    using Type = frontend::ast::ASTNode::Type;
    switch (node->type) {
    case Type::CompUnit:
        processCompUnit(*std::static_pointer_cast<frontend::ast::CompUnit>(node));
        break;
    default:
        break;
    }
    return std::move(module);
}

std::unique_ptr<IRModule> IRBuilder::build(const frontend::ast::CompUnit& compUnit) {
    processCompUnit(compUnit);
    return std::move(module);
}

void IRBuilder::processCompUnit(const frontend::ast::CompUnit& compUnit) {
    for (const auto& declOrFunc : compUnit.decls) {
        // declOrFunc 可能是 Decl 或 FuncDef
        if (!declOrFunc) continue;
        if (declOrFunc->type == frontend::ast::ASTNode::Type::Decl) {
            processDecl(*std::static_pointer_cast<frontend::ast::Decl>(declOrFunc));
        } else if (declOrFunc->type == frontend::ast::ASTNode::Type::FuncDef) {
            processFuncDef(*std::static_pointer_cast<frontend::ast::FuncDef>(declOrFunc));
        }
    }
}

void IRBuilder::processDecl(const frontend::ast::Decl& decl) {
    if (!decl.decl) return;
    if (decl.decl->type == frontend::ast::ASTNode::Type::ConstDecl) {
        auto constDecl = std::static_pointer_cast<frontend::ast::ConstDecl>(decl.decl);
        for (const auto& constDefNode : constDecl->constDefs) {
            auto constDef = std::static_pointer_cast<frontend::ast::ConstDef>(constDefNode);
            std::string name = constDef->ident;
            int value = 0;
            // 这里只处理常量初值为单一常量表达式的情况
            if (constDef->constInitVal) {
                // 递归处理constInitVal
                // 这里只做简单处理，实际应递归求值
            }
            auto* irValue = createValue(name, IRValue::ValueType::CONSTANTINT_VALUE);
            irValue->constant = value;
        }
    } else if (decl.decl->type == frontend::ast::ASTNode::Type::VarDecl) {
        auto varDecl = std::static_pointer_cast<frontend::ast::VarDecl>(decl.decl);
        for (const auto& varDefNode : varDecl->varDefs) {
            auto varDef = std::static_pointer_cast<frontend::ast::VarDef>(varDefNode);
            std::string name = varDef->ident;
            auto* irValue = createValue(name, IRValue::ValueType::TEMP_VALUE);
            if (varDef->initVal && *varDef->initVal) {
                auto initValNode = *varDef->initVal;
                if (initValNode->type == frontend::ast::ASTNode::Type::InitVal) {
                    auto initVal = std::static_pointer_cast<frontend::ast::InitVal>(initValNode);
                    // 这里只处理简单表达式
                    // 可递归处理initVal
                }
            }
        }
    }
}

void IRBuilder::processFuncDef(const frontend::ast::FuncDef& funcDef) {
    currentFunction = module->addFunction(funcDef.ident);
    currentBlock = currentFunction->addBlock("entry");
    // 处理参数
    if (funcDef.funcFParams) {
        auto params = std::static_pointer_cast<frontend::ast::FuncFParams>(funcDef.funcFParams);
        for (const auto& paramNode : params->funcFParams) {
            auto param = std::static_pointer_cast<frontend::ast::FuncFParam>(paramNode);
            createValue(param->ident, IRValue::ValueType::ARGUMENT_VALUE);
        }
    }
    // 处理函数体
    processBlock(*std::static_pointer_cast<frontend::ast::Block>(funcDef.block));
}

void IRBuilder::processBlock(const frontend::ast::Block& block) {
    for (const auto& itemNode : block.blockItems) {
        if (!itemNode) continue;
        auto blockItem = std::static_pointer_cast<frontend::ast::BlockItem>(itemNode);
        if (!blockItem->item) continue;
        if (blockItem->item->type == frontend::ast::ASTNode::Type::Decl) {
            processDecl(*std::static_pointer_cast<frontend::ast::Decl>(blockItem->item));
        } else if (blockItem->item->type == frontend::ast::ASTNode::Type::Stmt) {
            processStmt(*std::static_pointer_cast<frontend::ast::Stmt>(blockItem->item));
        }
    }
}

void IRBuilder::processStmt(const frontend::ast::Stmt& stmt) {
    using Type = frontend::ast::ASTNode::Type;
    if (!stmt.stmt) return;
    auto node = stmt.stmt;
    switch (node->type) {
    case Type::ExpStmt: {
        auto expStmt = std::static_pointer_cast<frontend::ast::Stmt::ExpStmt>(node);
        if (expStmt->exp && *expStmt->exp) {
            processExp(*std::static_pointer_cast<frontend::ast::Exp>(*expStmt->exp));
        }
        break;
    }
    case Type::IfStmt: {
        auto ifStmt = std::static_pointer_cast<frontend::ast::Stmt::IfStmt>(node);
        processExp(*std::static_pointer_cast<frontend::ast::Exp>(ifStmt->exp));
        processBlock(*std::static_pointer_cast<frontend::ast::Block>(ifStmt->block));
        if (ifStmt->elseStmt && *ifStmt->elseStmt) {
            processStmt(*std::static_pointer_cast<frontend::ast::Stmt>(*ifStmt->elseStmt));
        }
        break;
    }
    case Type::ReturnStmt: {
        auto retStmt = std::static_pointer_cast<frontend::ast::Stmt::ReturnStmt>(node);
        if (retStmt->exp && *retStmt->exp) {
            processExp(*std::static_pointer_cast<frontend::ast::Exp>(*retStmt->exp));
        }
        break;
    }
    default:
        break;
    }
}

IRValue* IRBuilder::processExp(const frontend::ast::Exp& exp) {
    // 这里只做最基础的递归，具体实现需根据AST结构完善
    if (exp.addExp) {
        // 这里只处理PrimaryExp->Number
        auto addExp = exp.addExp;
        // 递归处理addExp
    }
    return nullptr;
}

IRValue* IRBuilder::createValue(const std::string& name, IRValue::ValueType type) {
    auto* value = new IRValue(type, name);
    symbolTable[name] = value;
    return value;
}

IRValue* IRBuilder::getValue(const std::string& name) {
    auto it = symbolTable.find(name);
    if (it != symbolTable.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace IR