#pragma once
#include <set>
#include <variant>
#include <vector>
#include <string>
#include <memory>
#include "RiscVOperand/OpRegister.h"
#include "RiscVOperand/OpImmediate.h"
#include <unordered_set>
#include "RiscVOperand/OpLabel.h"

namespace backend {


using pImmediate = std::shared_ptr<Immediate>;
using AnyRegister = std::shared_ptr<Operand>;  
using OperandVariant = std::variant<
    AnyRegister,    
    pImmediate,
    std::shared_ptr<Label>
>;

enum class InstType {
    R,
    I, 
    S,
    B,
    U,
    J,
    Pseudo
};

enum class InstructionTy {
    NOP,
    // RV32I
    LUI, AUIPC, JAL, JALR,
    BEQ, BNE, BLT, BGE, BLTU, BGEU,
    LB, LH, LW, LBU, LHU, SB, SH, SW,
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
    // RV64I
    LWU, LD, SD, ADDIW, SLLIW, SRLIW, SRAIW,
    ADDW, SUBW, SLLW, SRLW, SRAW,
    // RV32M
    MUL, MULH, MULHSU, MULHU, DIV, DIVU, REM, REMU,
    // RV64M 
    MULW, DIVW, DIVUW, REMW, REMUW,
    // RV32F
    FLW, FSW, FMADD_S, FMSUB_S, FNMADD_S, FNMSUB_S,
    FADD_S, FSUB_S, FMUL_S, FDIV_S, FSQRT_S,
    FSGNJ_S, FSGNJN_S, FSGNJX_S, FMIN_S, FMAX_S,
    FCVT_W_S, FCVT_WU_S, FMV_X_W, FEQ_S, FLT_S, FLE_S,
    FCLASS_S, FCVT_S_W, FCVT_S_WU, FMV_W_X,
    // RV64F
    FCVT_L_S, FCVT_LU_S, FCVT_S_L, FCVT_S_LU,
    // RV32D + RV64D
    FLD, FSD, FMV_X_D, FMV_D_X,
    // 伪指�?
    CALL, RET, J, MV, FMV_S, FNEG_S, SEXT_W, ZEXT_W, LI, LA,
    CMOV,    
    NOT,     
    SEQZ,    
    SNEZ,    
};

inline const char* RiscVTypeName(InstructionTy ty);

class Instruction {
public:
    virtual InstType getInstType() const { return InstType::R; } 
    virtual OperandVariant getOperand(int) const { return OperandVariant{}; }
    virtual void setOperand(int, OperandVariant) {}  
    virtual bool isLoad() const { return false; } 
    virtual bool isStore() const { return false; } 
    virtual bool isBranch() const { return false; }  
    virtual bool isCall() const { return false; } 
    virtual bool isReturn() const { return false; }  
    virtual std::string output() const { return ""; }  

protected:
    std::vector<std::shared_ptr<Operand>> regDef, regUse;

    void reg_def_push_back(std::shared_ptr<Operand> reg) {
        regDef.push_back(reg);
        if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
            vreg->defUsers.insert(this);
        }
    }
    
    void reg_use_push_back(std::shared_ptr<Operand> reg) {
        regUse.push_back(reg);
        if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
            vreg->useUsers.insert(this);
        }
    }

public:
    virtual ~Instruction() {
        for (auto reg : regDef) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                vreg->defUsers.erase(this);
            }
        }
        for (auto reg : regUse) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                vreg->useUsers.erase(this);
            }
        }
    }

    std::vector<std::shared_ptr<Operand>> getDefRegisters() const { return regDef; }
    std::vector<std::shared_ptr<Operand>> getUseRegisters() const { return regUse; }
    std::set<rRegister> liveOut;
    template<typename OldRegType, typename NewRegType>
    void replaceRegisterUse(OldRegType oldReg, NewRegType newReg) {
        for (auto& reg : regUse) {
            if (reg == oldReg) {
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(oldReg)) {
                    vreg->useUsers.erase(this);
                }
                reg = newReg;
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(newReg)) {
                    vreg->useUsers.insert(this);
                }
            }
        }
    }

    template<typename OldRegType, typename NewRegType>
    void replaceRegisterDef(OldRegType oldReg, NewRegType newReg) {
        for (auto& reg : regDef) {
            if (reg == oldReg) {
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(oldReg)) {
                    vreg->defUsers.erase(this);
                }
                reg = newReg;
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(newReg)) {
                    vreg->defUsers.insert(this);
                }
            }
        }
    }

    virtual void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) {
        for (auto& reg : regDef) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)){
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) {
                    replaceRegisterDef(reg, it->second);
                    reg = it->second;
                }
            }
        }
        
        for (auto& reg : regUse) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) {
                    replaceRegisterUse(reg, it->second);
                    reg = it->second;
                }
            }
        }
    }
};

// R-type
class RInstruction : public Instruction {
    InstructionTy ty;
    AnyRegister rd, rs1, rs2;
public:
    RInstruction(InstructionTy ty, AnyRegister rd, AnyRegister rs1, AnyRegister rs2)
        : ty(ty), rd(rd), rs1(rs1), rs2(rs2) {
        reg_def_push_back(rd);
        reg_use_push_back(rs1);
        if (rs2) reg_use_push_back(rs2);
    }
    InstType getInstType() const override { return InstType::R; }
    bool isLoad() const override { 
        return false; 
    }
    bool isStore() const override { return false; }
    bool isBranch() const override { return false; }
    bool isCall() const override { return ty == InstructionTy::CALL; }
    bool isReturn() const override { return ty == InstructionTy::RET; }
    
    std::string output() const override {
        if (!rs2) {
            return std::string(RiscVTypeName(ty)) + " " +
                   rd->toString() + ", " +
                   rs1->toString() + "\n";
        }
        return std::string(RiscVTypeName(ty)) + " " +
               rd->toString() + ", " +
               rs1->toString() + ", " +
               rs2->toString() + "\n";
    }

    OperandVariant getOperand(int index) const override {
        switch(index) {
            case 0: return rd;
            case 1: return rs1;
            case 2: return rs2;
            default: throw ("Invalid operand index");
        }
    }

    void setOperand(int index, OperandVariant operand) override {
        auto reg = std::get<AnyRegister>(operand);
        switch(index) {
            case 0: replaceRegisterDef(rd, reg); rd = reg; break;
            case 1: replaceRegisterUse(rs1, reg); rs1 = reg; break;
            case 2: replaceRegisterUse(rs2, reg); rs2 = reg; break;
            default: throw ("Invalid operand index");
        }
    }

    void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        auto tryReplace = [&](AnyRegister& reg) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) reg = it->second;
            }
        };
        tryReplace(rd);
        tryReplace(rs1);
        if (rs2) tryReplace(rs2);
    }
};

class InstructionList {
private:
    std::vector<std::shared_ptr<Instruction>> instructions;

public:
    void addInstruction(std::shared_ptr<Instruction> inst) {
        instructions.push_back(inst);
    }

    void insertInstruction(size_t index, std::shared_ptr<Instruction> inst) {
        if(index > instructions.size()) {
            throw ("Invalid insert position");
        }
        instructions.insert(instructions.begin() + index, inst);
    }

    std::vector<std::shared_ptr<Instruction>>& getInstructions() {
        return instructions;
    }

    void clear() {
        instructions.clear();
    }
};

// rd = rs1 op imm(12)
class IInstruction : public Instruction {
    InstructionTy ty;
    AnyRegister rd, rs1;
    pImmediate imm;
public:
    IInstruction(InstructionTy ty, AnyRegister rd, AnyRegister rs1, pImmediate imm)
        : ty(ty), rd(rd), rs1(rs1), imm(imm) {
        reg_def_push_back(rd);
        reg_use_push_back(rs1);
    }
    InstType getInstType() const override { return InstType::I; }
    
    bool isLoad() const override { 
        return (ty >= InstructionTy::LB && ty <= InstructionTy::LHU) ||
            ty == InstructionTy::LWU ||
            ty == InstructionTy::LD ||
            ty == InstructionTy::FLW ||
            ty == InstructionTy::FLD;
    }

    bool isStore() const override { return false; }
    bool isBranch() const override { return ty == InstructionTy::JALR; }
    bool isCall() const override { return false; }
    bool isReturn() const override { return false; }
    
    std::string output() const override {
        return isLoad() ? std::string(RiscVTypeName(ty))  + " " +
               rd->toString() + ", " + imm->toString() + "(" + rs1->toString() + ")\n"
               : std::string(RiscVTypeName(ty)) + " " +
               rd->toString() + ", " + rs1->toString() + ", " + imm->toString() + "\n";
    }

    OperandVariant getOperand(int index) const override {
        switch(index) {
            case 0: return rd;
            case 1: return rs1;
            case 2: return imm;
            default: throw ("Invalid operand index");
        }
    }

    void setOperand(int index, OperandVariant operand) override {
        if (index == 0) { 
            auto reg = std::get<AnyRegister>(operand); 
            replaceRegisterDef(rd, reg); 
            rd = reg; 
        }
        else if (index == 1) { 
            auto reg = std::get<AnyRegister>(operand); 
            replaceRegisterUse(rs1, reg); 
            rs1 = reg; 
        }
        else if (index == 2) imm = std::get<pImmediate>(operand);
        else throw ("Invalid operand index");
    }

    void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        // 首先调用基类方法更新regDef和regUse向量
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        
        auto tryReplace = [&](AnyRegister& reg) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) reg = it->second;
            }
        };
        tryReplace(rd);
        tryReplace(rs1);
    }
};

// S-type
class SInstruction : public Instruction {
    InstructionTy ty;
    AnyRegister rs1, rs2;
    pImmediate imm;
public:
    SInstruction(InstructionTy ty, AnyRegister rs1, AnyRegister rs2, pImmediate imm)
        : ty(ty), rs1(rs1), rs2(rs2), imm(imm) {
        reg_use_push_back(rs1);
        if (rs2) reg_use_push_back(rs2);
    }
    InstType getInstType() const override { return InstType::S; }
    
    bool isLoad() const override { return false; }
    bool isStore() const override { return true; }
    bool isBranch() const override { return false; }
    bool isCall() const override { return false; }
    bool isReturn() const override { return false; }
    
    std::string output() const override {
        return std::string(RiscVTypeName(ty)) + " "
           + rs2->toString() + ", "
           + imm->toString() + "(" + rs1->toString() + ")\n";
    }

    OperandVariant getOperand(int index) const override {
        switch(index) {
            case 0: return rs1;
            case 1: return rs2;
            case 2: return imm;
            default: throw ("Invalid operand index");
        }
    }

    void setOperand(int index, OperandVariant operand) override {
        if (index == 0) { auto reg = std::get<AnyRegister>(operand); replaceRegisterUse(rs1, reg); rs1 = reg; }
        else if (index == 1) { auto reg = std::get<AnyRegister>(operand); replaceRegisterUse(rs2, reg); rs2 = reg; }
        else if (index == 2) imm = std::get<pImmediate>(operand);
        else throw ("Invalid operand index");
    }

    void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        // 首先调用基类方法更新regDef和regUse向量
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        
        auto tryReplace = [&](AnyRegister& reg) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) reg = it->second;
            }
        };
        tryReplace(rs1);
        if (rs2) tryReplace(rs2);
    }
};

// B-type指令实现
class BInstruction : public Instruction {
    InstructionTy ty;
    AnyRegister rs1, rs2;
    std::shared_ptr<Label> label; 
public:
    BInstruction(InstructionTy ty, AnyRegister rs1, AnyRegister rs2, std::shared_ptr<Label> label)
        : ty(ty), rs1(std::move(rs1)), rs2(std::move(rs2)), label(std::move(label)) {
        reg_use_push_back(this->rs1);
        reg_use_push_back(this->rs2);
    }
    InstType getInstType() const override { return InstType::B; }
    
    bool isLoad() const override { return false; }
    bool isStore() const override { return false; }
    bool isBranch() const override { return true; }
    bool isCall() const override { return false; }
    bool isReturn() const override { return false; }
    
    std::string output() const override {
        return std::string(RiscVTypeName(ty)) + " "
        + rs1->toString() + ", "
        + rs2->toString() + ", "
        + label->toString() + "\n";
    }

    OperandVariant getOperand(int index) const override {
        switch(index) {
            case 0: return rs1;
            case 1: return rs2;
            case 2: return label;
            default: throw ("Invalid operand index");
        }
    }

    void setOperand(int index, OperandVariant operand) override {
        switch(index) {
            case 0: 
                replaceRegisterUse(rs1, std::get<AnyRegister>(operand));
                rs1 = std::get<AnyRegister>(operand); 
                break;
            case 1: 
                replaceRegisterUse(rs2, std::get<AnyRegister>(operand));
                rs2 = std::get<AnyRegister>(operand); 
                break;
            case 2: label = std::get<std::shared_ptr<Label>>(operand); break;
            default: throw ("Invalid operand index");
        }
    }

    void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        // 首先调用基类方法更新regDef和regUse向量
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        
        // 然后更新rs1、rs2成员变量
        auto tryReplace = [&](AnyRegister& reg) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) reg = it->second;
            }
        };
        tryReplace(rs1);
        if (rs2) tryReplace(rs2);
    }
};

// U-type指令实现
class UInstruction : public Instruction {
    InstructionTy ty;
    AnyRegister rd;
    pImmediate imm;
public:
    UInstruction(InstructionTy ty, AnyRegister rd, pImmediate imm)
        : ty(ty), rd(std::move(rd)), imm(std::move(imm)) {
        reg_def_push_back(this->rd);
    }
        
    InstType getInstType() const override { return InstType::U; }
    
    bool isLoad() const override { return false; }
    bool isStore() const override { return false; }
    bool isBranch() const override { return false; }
    bool isCall() const override { return false; }
    bool isReturn() const override { return false; }
    
    std::string output() const override {
        return std::string(RiscVTypeName(ty)) + " "
        + rd->toString() + ", "
        + imm->toString() + "\n";
    }

    OperandVariant getOperand(int index) const override {
        switch(index) {
            case 0: return rd;
            case 1: return imm;
            default: throw ("Invalid operand index");
        }
    }

    void setOperand(int index, OperandVariant operand) override {
        switch(index) {
            case 0: rd = std::get<AnyRegister>(operand); break;
            case 1: imm = std::get<pImmediate>(operand); break;
            default: throw ("Invalid operand index");
        }
    }

    void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        // 首先调用基类方法更新regDef和regUse向量
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        
        auto tryReplace = [&](AnyRegister& reg) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) reg = it->second;
            }
        };
        tryReplace(rd);
    }
};

// J-type指令实现
class JInstruction : public Instruction {
    InstructionTy ty;
    AnyRegister rd;
    std::shared_ptr<Label> label;  
public:
    JInstruction(InstructionTy ty, AnyRegister rd, std::shared_ptr<Label> label)
        : ty(ty), rd(std::move(rd)), label(std::move(label)) {
        reg_def_push_back(this->rd);
    }
        
    InstType getInstType() const override { return InstType::J; }
    
    bool isLoad() const override { return false; }
    bool isStore() const override { return false; }
    bool isBranch() const override { return true; }
    bool isCall() const override { return ty == InstructionTy::JAL; }
    bool isReturn() const override { return false; }
    
    std::string output() const override {
        if (ty == InstructionTy::JALR) {
            // JALR指令格式: jalr rd, offset(rs1)
            return std::string(RiscVTypeName(ty)) + " "
                + rd->toString() + ", "
                + label->toString() + "\n";
        } else {
            // J指令格式: jal rd, offset
            return std::string(RiscVTypeName(ty)) + " "
                + rd->toString() + ", "
                + label->toString() + "\n";
        }
    }

    OperandVariant getOperand(int index) const override {
        switch(index) {
            case 0: return rd;
            case 1: return label;
            default: throw ("Invalid operand index");
        }
    }

    void setOperand(int index, OperandVariant operand) override {
        switch(index) {
            case 0: rd = std::get<AnyRegister>(operand); break;
            case 1: label = std::get<std::shared_ptr<Label>>(operand); break;
            default: throw ("Invalid operand index");
        }
    }
     void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        
        auto tryReplace = [&](AnyRegister& reg) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) reg = it->second;
            }
        };
        tryReplace(rd);
    }
};

// 伪指�?
class PseudoInstruction : public Instruction {
    InstructionTy ty;
    std::vector<OperandVariant> operands;
public:
    PseudoInstruction(InstructionTy ty, std::vector<OperandVariant> operands)
        : ty(ty), operands(std::move(operands)) {
        auto pushDef = [&](size_t index) {
            if (index < this->operands.size() && std::holds_alternative<AnyRegister>(this->operands[index])) {
                reg_def_push_back(std::get<AnyRegister>(this->operands[index]));
            }
        };
        auto pushUse = [&](size_t index) {
            if (index < this->operands.size() && std::holds_alternative<AnyRegister>(this->operands[index])) {
                reg_use_push_back(std::get<AnyRegister>(this->operands[index]));
            }
        };

        switch (ty) {
            case InstructionTy::MV:
            case InstructionTy::FMV_S:
            case InstructionTy::LI:
            case InstructionTy::LA:
            case InstructionTy::SEQZ:
            case InstructionTy::SNEZ:
            case InstructionTy::NOT:
            case InstructionTy::SEXT_W:
            case InstructionTy::ZEXT_W:
            case InstructionTy::FNEG_S:
                pushDef(0);
                pushUse(1);
                break;
            case InstructionTy::J:
                break;
            case InstructionTy::RET:
                break;
            default:
                for (size_t i = 0; i < this->operands.size(); ++i) {
                    pushUse(i);
                }
                break;
        }
    }
        
    InstType getInstType() const override { return InstType::Pseudo; }
    
    bool isLoad() const override { return false; }
    bool isStore() const override { return false; }
    bool isBranch() const override { 
        return ty == InstructionTy::J || ty == InstructionTy::RET; 
    }
    bool isCall() const override { return ty == InstructionTy::CALL; }
    bool isReturn() const override { return ty == InstructionTy::RET; }
    


    std::string output() const override {
        std::string result = RiscVTypeName(ty);
        bool first = true;
        for (const auto& op : operands) {
            result += first ? " " : ", ";
            result += std::visit([](auto&& arg) { return arg->toString(); }, op);
            first = false;
        }
        return result + "\n";
    }

    OperandVariant getOperand(int index) const override {
        if (index < 0 || index >= operands.size()) {
            throw ("Invalid operand index");
        }
        return operands[index];
    }

    void setOperand(int index, OperandVariant operand) override {
        if (index < 0 || index >= operands.size()) {
            throw ("Invalid operand index");
        }
        operands[index] = operand;
    }

    void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        for (auto& op : operands) {
            if (std::holds_alternative<AnyRegister>(op)) {
                auto& reg = std::get<AnyRegister>(op);
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                    auto it = vregToPregMap.find(vreg);
                    if (it != vregToPregMap.end()) {
                        reg = it->second;
                        op = reg; // 更新variant
                    }
                }
            }
        }
    }
};

class Call : public Instruction {
    std::shared_ptr<Label> callee;
    std::vector<AnyRegister> args;
    AnyRegister retReg;
public:
    Call(std::shared_ptr<Label> callee, const std::vector<AnyRegister>& args, AnyRegister retReg = nullptr)
        : callee(callee), args(args), retReg(retReg) {
        for (auto arg : args) {
            reg_use_push_back(arg);
        }
        if (retReg) {
            reg_def_push_back(retReg);
        }
    }
    
    InstType getInstType() const override { return InstType::Pseudo; }
    
    bool isCall() const override { return true; }
    
    std::string output() const override {
        return "call " + callee->toString() + "\n";
    }
    
    OperandVariant getOperand(int index) const override {
        if (index == 0) return callee;
        if (retReg && index == args.size() + 1) return retReg;
        if (index > 0 && index <= args.size()) return args[index-1];
        throw ("Invalid operand index");
    }
    
    void setOperand(int index, OperandVariant operand) override {
        if (index == 0) callee = std::get<std::shared_ptr<Label>>(operand);
        else if (retReg && index == args.size() + 1) {
            auto reg = std::get<AnyRegister>(operand);
            replaceRegisterDef(retReg, reg);
            retReg = reg;
        }
        else if (index > 0 && index <= args.size()) {
            auto reg = std::get<AnyRegister>(operand);
            replaceRegisterUse(args[index-1], reg);
            args[index-1] = reg;
        }
        else throw ("Invalid operand index");
    }

     void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        for (auto& arg : args) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(arg)) {
                auto it = vregToPregMap.find(vreg);
                if (it != vregToPregMap.end()) {
                    arg = it->second;
                }
            }
        }
    }
};


inline const char* RiscVTypeName(InstructionTy ty) {
    switch(ty) {
        case InstructionTy::NOP: return "nop";
        // RV32I
        case InstructionTy::LUI: return "lui";
        case InstructionTy::AUIPC: return "auipc";
        case InstructionTy::JAL: return "jal";
        case InstructionTy::JALR: return "jalr";
        case InstructionTy::BEQ: return "beq";
        case InstructionTy::BNE: return "bne";
        case InstructionTy::BLT: return "blt";
        case InstructionTy::BGE: return "bge";
        case InstructionTy::BLTU: return "bltu";
        case InstructionTy::BGEU: return "bgeu";
        case InstructionTy::LB: return "lb";
        case InstructionTy::LH: return "lh";
        case InstructionTy::LW: return "lw";
        case InstructionTy::LBU: return "lbu";
        case InstructionTy::LHU: return "lhu";
        case InstructionTy::SB: return "sb";
        case InstructionTy::SH: return "sh";
        case InstructionTy::SW: return "sw";
        case InstructionTy::ADDI: return "addi";
        case InstructionTy::SLTI: return "slti";
        case InstructionTy::SLTIU: return "sltiu";
        case InstructionTy::XORI: return "xori";
        case InstructionTy::ORI: return "ori";
        case InstructionTy::ANDI: return "andi";
        case InstructionTy::SLLI: return "slli";
        case InstructionTy::SRLI: return "srli";
        case InstructionTy::SRAI: return "srai";
        case InstructionTy::ADD: return "add";
        case InstructionTy::SUB: return "sub";
        case InstructionTy::SLL: return "sll";
        case InstructionTy::SLT: return "slt";
        case InstructionTy::SLTU: return "sltu";
        case InstructionTy::XOR: return "xor";
        case InstructionTy::SRL: return "srl";
        case InstructionTy::SRA: return "sra";
        case InstructionTy::OR: return "or";
        case InstructionTy::AND: return "and";
        // RV64I
        case InstructionTy::LWU: return "lwu";
        case InstructionTy::LD: return "ld";
        case InstructionTy::SD: return "sd";
        case InstructionTy::ADDIW: return "addiw";
        case InstructionTy::SLLIW: return "slliw";
        case InstructionTy::SRLIW: return "srliw";
        case InstructionTy::SRAIW: return "sraiw";
        case InstructionTy::ADDW: return "addw";
        case InstructionTy::SUBW: return "subw";
        case InstructionTy::SLLW: return "sllw";
        case InstructionTy::SRLW: return "srlw";
        case InstructionTy::SRAW: return "sraw";
        // RV32M
        case InstructionTy::MUL: return "mul";
        case InstructionTy::MULH: return "mulh";
        case InstructionTy::MULHSU: return "mulhsu";
        case InstructionTy::MULHU: return "mulhu";
        case InstructionTy::DIV: return "div";
        case InstructionTy::DIVU: return "divu";
        case InstructionTy::REM: return "rem";
        case InstructionTy::REMU: return "remu";
        // RV64M
        case InstructionTy::MULW: return "mulw";
        case InstructionTy::DIVW: return "divw";
        case InstructionTy::DIVUW: return "divuw";
        case InstructionTy::REMW: return "remw";
        case InstructionTy::REMUW: return "remuw";
        case InstructionTy::FLW: return "flw";
        case InstructionTy::FSW: return "fsw";
        case InstructionTy::FADD_S: return "fadd.s";
        case InstructionTy::FSUB_S: return "fsub.s";
        case InstructionTy::FMUL_S: return "fmul.s";
        case InstructionTy::FDIV_S: return "fdiv.s";
        case InstructionTy::FSGNJ_S: return "fsgnj.s";
        case InstructionTy::FSGNJN_S: return "fsgnjn.s";
        case InstructionTy::FEQ_S: return "feq.s";
        case InstructionTy::FLT_S: return "flt.s";
        case InstructionTy::FLE_S: return "fle.s";
        case InstructionTy::FCVT_W_S: return "fcvt.w.s";
        case InstructionTy::FCVT_S_W: return "fcvt.s.w";
        case InstructionTy::FMV_W_X: return "fmv.w.x";
        // 伪指�?
        case InstructionTy::CALL: return "call";
        case InstructionTy::RET: return "ret";
        case InstructionTy::J: return "j";
        case InstructionTy::MV: return "mv";
        case InstructionTy::NOT: return "not";
        case InstructionTy::SEQZ: return "seqz";
        case InstructionTy::SNEZ: return "snez";
        case InstructionTy::FMV_S: return "fmv.s";
        case InstructionTy::FNEG_S: return "fneg.s";
        case InstructionTy::SEXT_W: return "sext.w";
        case InstructionTy::ZEXT_W: return "zext.w";
        case InstructionTy::LI: return "li";
        case InstructionTy::LA: return "la";
        default: return "unknown";
    }
}

inline std::shared_ptr<Instruction> createPseudoInstruction(InstructionTy ty, 
    const std::vector<OperandVariant>& operands) {
    return std::make_shared<PseudoInstruction>(ty, operands);
}

} // namespace backend


