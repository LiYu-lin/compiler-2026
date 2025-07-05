// IRBuilder.h
#include "IR_Value.h"
#include "Parse/AST.h"
#include <memory>
#include <unordered_map>

namespace IR {
    class IRBuilder {
        std::unique_ptr<IRModule> module;
        IRFunction* currentFunction = nullptr;
        IRBlock* currentBlock = nullptr;
        std::unordered_map<std::string, IRValue*> symbolTable;

    public:
        IRBuilder() : module(std::make_unique<IRModule>()) {}

        // 新增支持ASTNodePtr的build接口
        std::unique_ptr<IRModule> build(const frontend::ast::ASTNodePtr& node);

        // 保持原有接口
        std::unique_ptr<IRModule> build(const frontend::ast::CompUnit& compUnit);

    private:
        void processCompUnit(const frontend::ast::CompUnit& compUnit);
        void processDecl(const frontend::ast::Decl& decl);
        void processFuncDef(const frontend::ast::FuncDef& funcDef);
        void processBlock(const frontend::ast::Block& block);
        void processStmt(const frontend::ast::Stmt& stmt);
        IRValue* processExp(const frontend::ast::Exp& exp);
        IRValue* createValue(const std::string& name, IRValue::ValueType type);
        IRValue* getValue(const std::string& name);
    };
}