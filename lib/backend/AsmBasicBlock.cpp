#include "component.h"
#include "instruction.h"
#include "Instructions.h"
#include "OpRegister.h"
#include <memory>
#include <cstring>

namespace backend {

namespace {

class EpilogueInstruction : public Instruction {
    AsmFunction* function;

public:
    explicit EpilogueInstruction(AsmFunction* function) : function(function) {}

    InstType getInstType() const override { return InstType::Pseudo; }

    std::string output() const override {
        auto fixedBytes = static_cast<int32_t>(function->getStackFrameSize());
        if (fixedBytes == 0) {
            return "";
        }

        std::string s;
        if (function->shouldSaveReturnAddress()) {
            s += "ld ra, " +
                 std::to_string(function->getReturnAddressOffsetFromFixedFrameBase()) +
                 "(sp)\n\t";
        }

        if (fixedBytes <= 2047) {
            s += "addi sp, sp, " + std::to_string(fixedBytes) + "\n";
        } else {
            s += "li t0, " + std::to_string(fixedBytes) + "\n\t";
            s += "add sp, sp, t0\n";
        }

        return s;
    }
};

class LoadStackArgumentInstruction : public Instruction {
    AsmFunction* function;
    AnyRegister rd;
    int argNo;
    bool isFloat;

public:
    LoadStackArgumentInstruction(AsmFunction* function, AnyRegister rd, int argNo, bool isFloat)
        : function(function), rd(rd), argNo(argNo), isFloat(isFloat) {
        reg_def_push_back(rd);
    }

    InstType getInstType() const override { return InstType::Pseudo; }
    bool isLoad() const override { return true; }

    std::string output() const override {
        auto offset = static_cast<int32_t>(
            function->getStackFrameSize() + static_cast<size_t>(argNo - 8) * 4);
        return std::string(isFloat ? "flw " : "lw ") +
               rd->toString() + ", " + std::to_string(offset) + "(sp)\n";
    }

    void replaceVRegsWithPhysRegs(const std::unordered_map<rRegister, pRegister>& vregToPregMap) override {
        Instruction::replaceVRegsWithPhysRegs(vregToPregMap);
        if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(rd)) {
            auto it = vregToPregMap.find(vreg);
            if (it != vregToPregMap.end()) {
                rd = it->second;
            }
        }
    }
};

AnyRegister materializeValue(AsmBasicBlock* block, IR::Value* value) {
    if (auto *arg = dynamic_cast<IR::Argument*>(value)) {
        if (arg->getArgNo() >= 8) {
            auto temp = VirtualRegister::createTemp(arg->getType()->isFloatTy());
            block->addInstruction(std::make_shared<LoadStackArgumentInstruction>(
                block->getParentFunction(), temp, arg->getArgNo(), arg->getType()->isFloatTy()));
            return temp;
        }
    }

    if (auto globalVar = dynamic_cast<IR::GlobalVariable*>(value)) {
        auto temp = VirtualRegister::createTemp(false);
        block->addInstruction(createPseudoInstruction(
            InstructionTy::LA,
            {temp, std::make_shared<Label>(sanitizeSymbolName(globalVar->getIRName()), true)}));
        return temp;
    }
    if (auto intConst = dynamic_cast<IR::ConstantInt32*>(value)) {
        if (intConst->getValue() == 0) {
            return VirtualRegister::createZero();
        }

        auto temp = VirtualRegister::createTemp(false);
        block->addInstruction(createPseudoInstruction(
            InstructionTy::LI,
            {temp, std::make_shared<Immediate>(intConst->getValue())}));
        return temp;
    }

    if (auto floatConst = dynamic_cast<IR::ConstantFloat*>(value)) {
        int32_t bits = 0;
        float raw = floatConst->getValue();
        std::memcpy(&bits, &raw, sizeof(bits));
        auto intTemp = VirtualRegister::createTemp(false);
        auto floatTemp = VirtualRegister::createTemp(true);
        block->addInstruction(createPseudoInstruction(
            InstructionTy::LI,
            {intTemp, std::make_shared<Immediate>(bits)}));
        block->addInstruction(std::make_shared<RInstruction>(
            InstructionTy::FMV_W_X,
            floatTemp,
            intTemp,
            nullptr));
        return floatTemp;
    }

    return VirtualRegister::create(value, value->getType()->isFloatTy());
}

void emitStackRestoreIfNeeded(AsmBasicBlock* block) {
    (void)block;
}

void emitFixedFrameRestoreIfNeeded(AsmBasicBlock* block) {
    block->addInstruction(std::make_shared<EpilogueInstruction>(
        block->getParentFunction()));
}

}


    std::vector<AsmBasicBlock*> AsmBasicBlock::getSuccessors() {
        std::vector<AsmBasicBlock*> successors;
            for (auto succ : irBlock->getSuccBlock()) {
        // 通过父函数的映射表找到对应的Asm基本�?
            if (parentFunction) {
                auto asmSucc = parentFunction->getAsmBasicBlock(succ);
                if (asmSucc) {
                    successors.push_back(asmSucc);
                }
            }
        }
        
        return successors;
    }

std::shared_ptr<Instruction> AsmBasicBlock::convertInstruction(IR::Instruction* inst) {
    if (!inst) return nullptr;

    int opcode = inst->getOpcode();
    // std::cout<< "Converting instruction: " << inst->getIRName() << " with opcode: " << opcode << std::endl;
    try {
        if (opcode >= IR::Instruction::BinaryBegin && opcode < IR::Instruction::BinaryEnd) {
            return convertBinaryInstruction(static_cast<IR::BinaryInstruction*>(inst));
        } 
        else if (opcode >= IR::Instruction::UnaryBegin && opcode < IR::Instruction::UnaryEnd) {
            return convertUnaryInstruction(static_cast<IR::UnaryInstruction*>(inst));
        } 
        else if (opcode >= IR::Instruction::CastBegin && opcode < IR::Instruction::CastEnd) {
            return convertCastInstruction(static_cast<IR::CastInstruction*>(inst));
        } 
        else if (opcode == IR::Instruction::Alloca) {
            return convertAllocaInstruction(static_cast<IR::AllocaInstruction*>(inst));
        } 
        else if (opcode == IR::Instruction::Load) {
            return convertLoadInstruction(static_cast<IR::LoadInstruction*>(inst));
        } 
        else if (opcode == IR::Instruction::Store) {
            return convertStoreInstruction(static_cast<IR::StoreInstruction*>(inst));
        } 
        else if (opcode == IR::Instruction::GEP) {
            return convertGEPInstruction(static_cast<IR::GetElementPtrInstruction*>(inst));
        } 
        else if (opcode == IR::Instruction::BR) {
            return convertBranchInstruction(static_cast<IR::BranchInstruction*>(inst));
        } 
        else if (opcode == IR::Instruction::Return) {
            return convertReturnInstruction(static_cast<IR::ReturnInstruction*>(inst));
        } 
        else if (opcode == IR::Instruction::Call) {
            return convertCallInstruction(static_cast<IR::CallInstruction*>(inst));
        } 
        else if (opcode == IR::Instruction::Phi) {
            return nullptr;
        } 
        else if (opcode >= IR::Instruction::CmpBegin && opcode < IR::Instruction::CmpEnd) {
            return convertCmpInstruction(static_cast<IR::CmpInstruction*>(inst));
        } 
        else {
            throw std::runtime_error("Unknown instruction, opcode: " + std::to_string(opcode));
        }
    } 
    catch (const std::exception& e) {
        std::cerr << "Error converting instruction: " << e.what() << std::endl;
        return nullptr;
    }
}


std::shared_ptr<Instruction> AsmBasicBlock::convertLoadInstruction(IR::LoadInstruction* inst) {
    if (auto *allocaInst = dynamic_cast<IR::AllocaInstruction*>(inst->getSrc())) {
        auto rd = VirtualRegister::create(inst, inst->getType()->isFloatTy());
        return std::make_shared<IInstruction>(
            inst->getType()->isFloatTy() ? InstructionTy::FLW : InstructionTy::LW,
            rd,
            VirtualRegister::createStackPointerRef(),
            std::make_shared<Immediate>(static_cast<int32_t>(parentFunction->getAllocaOffset(allocaInst))));
    }

    auto addr = materializeValue(this, inst->getSrc());
    auto rd = VirtualRegister::create(inst, inst->getType()->isFloatTy());
    
    return std::make_shared<IInstruction>(
        inst->getType()->isFloatTy() ? InstructionTy::FLW : InstructionTy::LW,
        rd, addr, std::make_shared<Immediate>(0));
}

std::shared_ptr<Instruction> AsmBasicBlock::convertStoreInstruction(IR::StoreInstruction* inst) {
    auto val = materializeValue(this, inst->getSrc());

    if (auto *allocaInst = dynamic_cast<IR::AllocaInstruction*>(inst->getDest())) {
        return std::make_shared<SInstruction>(
            inst->getSrc()->getType()->isFloatTy() ? InstructionTy::FSW : InstructionTy::SW,
            VirtualRegister::createStackPointerRef(),
            val,
            std::make_shared<Immediate>(static_cast<int32_t>(parentFunction->getAllocaOffset(allocaInst))));
    }

    auto addr = materializeValue(this, inst->getDest());

    return std::make_shared<SInstruction>(
        inst->getSrc()->getType()->isFloatTy() ? InstructionTy::FSW : InstructionTy::SW,
        addr, val, std::make_shared<Immediate>(0));
}

std::shared_ptr<Instruction> AsmBasicBlock::convertBinaryInstruction(IR::BinaryInstruction* inst) {
    auto lhs = materializeValue(this, inst->getOperand(0));
    auto rhs = materializeValue(this, inst->getOperand(1));
    auto rd = VirtualRegister::create(inst, inst->getType()->isFloatTy());
    
    switch (inst->getOpcode()) {
        // gzj：这里只处理i32，所以用32位w
        case IR::Instruction::Add:
            return std::make_shared<RInstruction>(InstructionTy::ADDW, rd, lhs, rhs);
        case IR::Instruction::Sub:
            return std::make_shared<RInstruction>(InstructionTy::SUBW, rd, lhs, rhs);
        case IR::Instruction::Mul:
            return std::make_shared<RInstruction>(InstructionTy::MULW, rd, lhs, rhs);
        case IR::Instruction::Div:
            return std::make_shared<RInstruction>(InstructionTy::DIVW, rd, lhs, rhs);
        case IR::Instruction::Rem:
            return std::make_shared<RInstruction>(InstructionTy::REMW, rd, lhs, rhs);
            
        // 浮点运算
        case IR::Instruction::FAdd:
            return std::make_shared<RInstruction>(InstructionTy::FADD_S, rd, lhs, rhs);
        case IR::Instruction::FSub:
            return std::make_shared<RInstruction>(InstructionTy::FSUB_S, rd, lhs, rhs);
        case IR::Instruction::FMul:
            return std::make_shared<RInstruction>(InstructionTy::FMUL_S, rd, lhs, rhs);
        case IR::Instruction::FDiv:
            return std::make_shared<RInstruction>(InstructionTy::FDIV_S, rd, lhs, rhs);
        // case IR::Instruction::FRem:
        //     // RISC-V没有直接的浮点取模指令，需要软件实�?
        //     addInstruction(std::make_shared<Call>(
        //         std::make_shared<Label>("fmodf"), 
        //         {lhs, rhs}, 
        //         rd));
        //     return nullptr;
            
        // 位运�?
        case IR::Instruction::And:
            return std::make_shared<RInstruction>(InstructionTy::AND, rd, lhs, rhs);
        case IR::Instruction::Or:
            return std::make_shared<RInstruction>(InstructionTy::OR, rd, lhs, rhs);
        case IR::Instruction::Xor:
            return std::make_shared<RInstruction>(InstructionTy::XOR, rd, lhs, rhs);
            
        default:
            throw std::runtime_error("Unsupported binary operation");
    }
}

std::shared_ptr<Instruction> AsmBasicBlock::convertGEPInstruction(IR::GetElementPtrInstruction* inst) {
    auto base = materializeValue(this, inst->getOperand(0));
    auto result = VirtualRegister::create(inst, inst->getType()->isFloatTy());
    std::vector<IR::Value*> indices = inst->getIndices();
    
    // 计算偏移�?
    auto offset = VirtualRegister::createTemp(false);
    addInstruction(std::make_shared<IInstruction>(
        InstructionTy::ADDI,
        offset,
        VirtualRegister::createZero(),
        std::make_shared<Immediate>(0)));
    
    for (size_t i = 0; i < indices.size(); ++i) {
        auto index = materializeValue(this, indices[i]);
        auto temp = VirtualRegister::createTemp(false);
        
        // 计算当前维度的偏�?
        if (i == 0) {
            // 第一维直接使用索�?
            addInstruction(std::make_shared<RInstruction>(
                InstructionTy::ADD,
                offset,
                offset,
                index));
        } else {
            auto size = VirtualRegister::createTemp(false);
            addInstruction(std::make_shared<IInstruction>(
                InstructionTy::ADDI,
                size,
                VirtualRegister::createZero(),
                std::make_shared<Immediate>(static_cast<int32_t>(
                    inst->getType()->isArrayTy() ? 
                    static_cast<const IR::ArrayType*>(inst->getType())->getElementType()->size() : 
                    inst->getType()->size()))));
            addInstruction(std::make_shared<RInstruction>(
                InstructionTy::MUL,
                temp,
                index,
                size));
                
            addInstruction(std::make_shared<RInstruction>(
                InstructionTy::ADD,
                offset,
                offset,
                temp));
        }
    }
    
    // 最终地址计算
    return std::make_shared<RInstruction>(
        InstructionTy::ADD,
        result,
        base,
        offset);
}

std::shared_ptr<Instruction> AsmBasicBlock::convertAllocaInstruction(IR::AllocaInstruction* inst) {
    IR::pType allocType = inst->getType()->getBase();
    int allocSize = allocType->size();
    allocSize = (allocSize + 3) / 4 * 4;
    parentFunction->recordStackAllocation(inst, allocSize, 4);
    return nullptr;
}

std::shared_ptr<Instruction> AsmBasicBlock::convertUnaryInstruction(IR::UnaryInstruction* inst) {
    auto src = materializeValue(this, inst->getOperand(0));
    auto dst = VirtualRegister::create(inst, inst->getType()->isFloatTy());

    switch (inst->getOpcode()) {
        case IR::Instruction::Neg:
            return std::make_shared<RInstruction>(
                InstructionTy::SUB, 
                dst, 
                VirtualRegister::createZero(), 
                src);
                
        case IR::Instruction::FNeg:
            return std::make_shared<RInstruction>(
                InstructionTy::FSGNJN_S,  // 符号位取�?
                dst,
                src,
                src);
                
        case IR::Instruction::Not:
            return std::make_shared<IInstruction>(
                InstructionTy::XORI,
                dst,
                src,
                std::make_shared<Immediate>(-1));  // 按位取反
                
        default:
            throw std::runtime_error("Unsupported unary operation");
    }
}

std::shared_ptr<Instruction> AsmBasicBlock::convertReturnInstruction(IR::ReturnInstruction* inst) {
    if (inst->isVoidReturn()) {
        emitStackRestoreIfNeeded(this);
        emitFixedFrameRestoreIfNeeded(this);
        return createPseudoInstruction(InstructionTy::RET, {});
    }

    auto retVal = materializeValue(this, inst->getOperand(0));
    auto retReg = VirtualRegister::createReturnValue(
        inst->operands[0]->getType()->isFloatTy());
    
    addInstruction(createPseudoInstruction(
        inst->getOperand(0)->getType()->isFloatTy() ? InstructionTy::FMV_S : InstructionTy::MV,
        {retReg, retVal}));

    emitStackRestoreIfNeeded(this);
    emitFixedFrameRestoreIfNeeded(this);
    return createPseudoInstruction(InstructionTy::RET, {});
}

std::shared_ptr<Instruction> AsmBasicBlock::convertBranchInstruction(IR::BranchInstruction* inst) {
    if (inst->isUnconditional()) {
        return createPseudoInstruction(
            InstructionTy::J,
            {std::make_shared<Label>(inst->getUnconditionalBlock()->getIRName())});
    }

    if (!inst->getTrueBlock() || !inst->getFalseBlock()) {
        throw std::runtime_error("Invalid conditional branch: missing target");
    }

    auto cond = materializeValue(this, inst->getCondition());
    auto trueLabel = inst->getTrueBlock()->getIRName();
    auto falseLabel = inst->getFalseBlock()->getIRName();

    addInstruction(std::make_shared<BInstruction>(
        InstructionTy::BNE,
        cond,
        VirtualRegister::createZero(),
        std::make_shared<Label>(trueLabel)));
    
    return createPseudoInstruction(
        InstructionTy::J,
        {std::make_shared<Label>(falseLabel)});
}

std::shared_ptr<Instruction> AsmBasicBlock::convertCastInstruction(IR::CastInstruction* inst) {
    auto src = materializeValue(this, inst->getOperand(0));
    auto dst = VirtualRegister::create(inst, inst->getType()->isFloatTy());

    switch (inst->getOpcode()) {
        case IR::Instruction::FPtoSI:
            return std::make_shared<RInstruction>(
                InstructionTy::FCVT_W_S,
                dst,
                src,
                nullptr);  
                
        case IR::Instruction::SItoFP:
            return std::make_shared<RInstruction>(
                InstructionTy::FCVT_S_W,
                dst,
                src,
                nullptr);
                
        default:
            throw std::runtime_error("Unsupported cast operation");
    }
}


std::shared_ptr<Instruction> AsmBasicBlock::convertCallInstruction(IR::CallInstruction* inst) {
    auto callArgs = inst->getArgs();

    for (size_t i = 0; i < callArgs.size() && i < 8; ++i) {
        auto src = materializeValue(this, callArgs[i]);
        AnyRegister dst = PhysicalRegister::getParamReg(static_cast<int>(i), callArgs[i]->getType()->isFloatTy());
        addInstruction(createPseudoInstruction(
            callArgs[i]->getType()->isFloatTy() ? InstructionTy::FMV_S : InstructionTy::MV,
            {dst, src}));
    }

    const int extraArgBytes = callArgs.size() > 8
        ? static_cast<int>((callArgs.size() - 8) * 4)
        : 0;
    const int intSaveOffset = static_cast<int>((extraArgBytes + 7) / 8 * 8);
    const int intSaveBytes = 64;
    const int floatSaveOffset = intSaveOffset + intSaveBytes;
    const int floatSaveBytes = 32;
    const int callFrameBytes = ((floatSaveOffset + floatSaveBytes + 15) / 16) * 16;

    addInstruction(std::make_shared<IInstruction>(
        InstructionTy::ADDI,
        VirtualRegister::createStackPointerRef(),
        VirtualRegister::createStackPointerRef(),
        std::make_shared<Immediate>(-callFrameBytes)));

    for (size_t i = 8; i < callArgs.size(); ++i) {
        auto src = materializeValue(this, callArgs[i]);
        addInstruction(std::make_shared<SInstruction>(
            callArgs[i]->getType()->isFloatTy() ? InstructionTy::FSW : InstructionTy::SW,
            VirtualRegister::createStackPointerRef(),
            src,
            std::make_shared<Immediate>(static_cast<int32_t>((i - 8) * 4))));
    }

    for (int reg = 5; reg <= 7; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::SD,
            VirtualRegister::createStackPointerRef(),
            PhysicalRegister::get(reg),
            std::make_shared<Immediate>(intSaveOffset + (reg - 5) * 8)));
    }
    for (int reg = 28; reg <= 31; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::SD,
            VirtualRegister::createStackPointerRef(),
            PhysicalRegister::get(reg),
            std::make_shared<Immediate>(intSaveOffset + (reg - 25) * 8)));
    }
    for (int reg = 5; reg <= 7; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::FSW,
            VirtualRegister::createStackPointerRef(),
            PhysicalRegister::get(reg, true),
            std::make_shared<Immediate>(floatSaveOffset + (reg - 5) * 4)));
    }
    for (int reg = 28; reg <= 31; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::FSW,
            VirtualRegister::createStackPointerRef(),
            PhysicalRegister::get(reg, true),
            std::make_shared<Immediate>(floatSaveOffset + 12 + (reg - 28) * 4)));
    }

    addInstruction(std::make_shared<Call>(
        std::make_shared<Label>(sanitizeSymbolName(inst->getCallee()->getIRName()), true),
        std::vector<AnyRegister>{},
        nullptr));

    for (int reg = 5; reg <= 7; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::LD,
            PhysicalRegister::get(reg),
            VirtualRegister::createStackPointerRef(),
            std::make_shared<Immediate>(intSaveOffset + (reg - 5) * 8)));
    }
    for (int reg = 28; reg <= 31; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::LD,
            PhysicalRegister::get(reg),
            VirtualRegister::createStackPointerRef(),
            std::make_shared<Immediate>(intSaveOffset + (reg - 25) * 8)));
    }
    for (int reg = 5; reg <= 7; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::FLW,
            PhysicalRegister::get(reg, true),
            VirtualRegister::createStackPointerRef(),
            std::make_shared<Immediate>(floatSaveOffset + (reg - 5) * 4)));
    }
    for (int reg = 28; reg <= 31; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::FLW,
            PhysicalRegister::get(reg, true),
            VirtualRegister::createStackPointerRef(),
            std::make_shared<Immediate>(floatSaveOffset + 12 + (reg - 28) * 4)));
    }
    addInstruction(std::make_shared<IInstruction>(
        InstructionTy::ADDI,
        VirtualRegister::createStackPointerRef(),
        VirtualRegister::createStackPointerRef(),
        std::make_shared<Immediate>(callFrameBytes)));

    if (!inst->getType()->isVoidTy()) {
        auto retReg = VirtualRegister::create(inst, inst->getType()->isFloatTy());
        addInstruction(createPseudoInstruction(
            inst->getType()->isFloatTy() ? InstructionTy::FMV_S : InstructionTy::MV,
            {retReg, PhysicalRegister::getParamReg(0, inst->getType()->isFloatTy())}));
    }

    return nullptr;
}
std::shared_ptr<Instruction> AsmBasicBlock::convertCmpInstruction(IR::CmpInstruction* inst) {
    auto lhs = materializeValue(this, inst->getOperand(0));
    auto rhs = materializeValue(this, inst->getOperand(1));
    auto dst = VirtualRegister::create(inst, inst->getType()->isFloatTy());

    if (auto icmp = dynamic_cast<IR::ICmpInstruction*>(inst)) {
        switch (icmp->getCmpCode()) {
            case IR::ICmpInstruction::Eq:
                addInstruction(std::make_shared<RInstruction>(InstructionTy::SUB, dst, lhs, rhs));
                return createPseudoInstruction(InstructionTy::SEQZ, {dst, dst});
            case IR::ICmpInstruction::Ne:
                addInstruction(std::make_shared<RInstruction>(InstructionTy::SUB, dst, lhs, rhs));
                return createPseudoInstruction(InstructionTy::SNEZ, {dst, dst});
            case IR::ICmpInstruction::Lt:
                return std::make_shared<RInstruction>(InstructionTy::SLT, dst, lhs, rhs);
            case IR::ICmpInstruction::Le:
                addInstruction(std::make_shared<RInstruction>(InstructionTy::SLT, dst, rhs, lhs));
                return std::make_shared<IInstruction>(InstructionTy::XORI, dst, dst, std::make_shared<Immediate>(1));
            case IR::ICmpInstruction::Gt:
                return std::make_shared<RInstruction>(InstructionTy::SLT, dst, rhs, lhs);
            case IR::ICmpInstruction::Ge:
                addInstruction(std::make_shared<RInstruction>(InstructionTy::SLT, dst, lhs, rhs));
                return std::make_shared<IInstruction>(InstructionTy::XORI, dst, dst, std::make_shared<Immediate>(1));
        }
    } 
    else if (auto fcmp = dynamic_cast<IR::FCmpInstruction*>(inst)) {
        switch (fcmp->getCmpCode()) {
            case IR::FCmpInstruction::FEq:
                return std::make_shared<RInstruction>(InstructionTy::FEQ_S, dst, lhs, rhs);
            case IR::FCmpInstruction::FNe:
                addInstruction(std::make_shared<RInstruction>(InstructionTy::FEQ_S, dst, lhs, rhs));
                return std::make_shared<IInstruction>(InstructionTy::XORI, dst, dst, std::make_shared<Immediate>(1));
            case IR::FCmpInstruction::FLt:
                return std::make_shared<RInstruction>(InstructionTy::FLT_S, dst, lhs, rhs);
            case IR::FCmpInstruction::FLe:
                return std::make_shared<RInstruction>(InstructionTy::FLE_S, dst, lhs, rhs);
            case IR::FCmpInstruction::FGt:
                return std::make_shared<RInstruction>(InstructionTy::FLT_S, dst, rhs, lhs);
            case IR::FCmpInstruction::FGe:
                return std::make_shared<RInstruction>(InstructionTy::FLE_S, dst, rhs, lhs);
        }
    }
    
    throw std::runtime_error("Unsupported comparison operation");
}



}



