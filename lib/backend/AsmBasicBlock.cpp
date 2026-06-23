#include "component.h"
#include "instruction.h"
#include "Instructions.h"
#include "OpRegister.h"
#include <algorithm>
#include <memory>
#include <cstring>
#include <unordered_map>

namespace backend {

namespace {

std::unordered_map<IR::Value*, size_t> addressValueSpillOffsets;

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
            auto offset = static_cast<int32_t>(function->getReturnAddressOffsetFromFixedFrameBase());
            if (fitsSigned12(offset)) {
                s += "ld ra, " + std::to_string(offset) + "(sp)\n\t";
            } else {
                s += "li t0, " + std::to_string(offset) + "\n\t";
                s += "add t0, sp, t0\n\t";
                s += "ld ra, 0(t0)\n\t";
            }
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
            function->getStackFrameSize() + static_cast<size_t>(argNo - 8) * 8);
        auto op = std::string(isFloat ? "flw " : "lw ");
        if (fitsSigned12(offset)) {
            return op + rd->toString() + ", " + std::to_string(offset) + "(sp)\n";
        }

        auto scratch = chooseScratchRegister({"sp"});
        std::string s;
        s += "li " + scratch + ", " + std::to_string(offset) + "\n\t";
        s += "add " + scratch + ", sp, " + scratch + "\n\t";
        s += op + rd->toString() + ", 0(" + scratch + ")\n";
        return s;
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
    if (auto *allocaInst = dynamic_cast<IR::AllocaInstruction*>(value)) {
        auto temp = VirtualRegister::createTemp(false);
        block->addInstruction(std::make_shared<IInstruction>(
            InstructionTy::ADDI,
            temp,
            PhysicalRegister::get(2), // 🚀 稳固：直接强制对接物理 sp 寄存器
            std::make_shared<Immediate>(static_cast<int32_t>(
                block->getParentFunction()->getAllocaOffset(allocaInst)))));
        return temp;
    }

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
    if (dynamic_cast<IR::GetElementPtrInstruction*>(value)) {
        auto it = addressValueSpillOffsets.find(value);
        if (it != addressValueSpillOffsets.end()) {
            auto temp = VirtualRegister::createTemp(false);
            block->addInstruction(std::make_shared<IInstruction>(
                InstructionTy::LD,
                temp,
                PhysicalRegister::get(2), // 🚀 稳固：直接强制对接物理 sp 寄存器
                std::make_shared<Immediate>(static_cast<int32_t>(it->second))));
            return temp;
        }
    }
    if (auto intConst = dynamic_cast<IR::ConstantInt32*>(value)) {
        if (intConst->getValue() == 0) {
            return PhysicalRegister::get(0); // 直接返回物理 zero 寄存器
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

void emitScalarConstantStore(AsmBasicBlock* block, AnyRegister base, IR::Constant* constant, int32_t offset) {
    if (auto *intConst = dynamic_cast<IR::ConstantInt32*>(constant)) {
        auto value = intConst->getValue() == 0 ? PhysicalRegister::get(0) : materializeValue(block, constant);
        block->addInstruction(std::make_shared<SInstruction>(
            InstructionTy::SW,
            base,
            value,
            std::make_shared<Immediate>(offset)));
        return;
    }

    if (auto *floatConst = dynamic_cast<IR::ConstantFloat*>(constant)) {
        auto value = materializeValue(block, floatConst);
        block->addInstruction(std::make_shared<SInstruction>(
            InstructionTy::FSW,
            base,
            value,
            std::make_shared<Immediate>(offset)));
    }
}

void emitConstantArrayStores(AsmBasicBlock* block, AnyRegister base, IR::Constant* constant, int32_t offset) {
    if (auto *array = dynamic_cast<IR::ConstantArray*>(constant)) {
        auto *arrayType = static_cast<const IR::ArrayType*>(array->getType());
        auto elementType = arrayType->getArrayBase();
        int32_t elementSize = static_cast<int32_t>(elementType->size());
        for (int i = 0; i < arrayType->getArraySize(); ++i) {
            emitConstantArrayStores(
                block,
                base,
                array->getOperand(static_cast<unsigned int>(i)),
                offset + i * elementSize);
        }
        return;
    }

    emitScalarConstantStore(block, base, constant, offset);
}

void emitConstantArrayStoresToAlloca(
    AsmBasicBlock* block,
    IR::AllocaInstruction* allocaInst,
    IR::Constant* constant,
    int32_t offset) {
    if (auto *array = dynamic_cast<IR::ConstantArray*>(constant)) {
        auto *arrayType = static_cast<const IR::ArrayType*>(array->getType());
        auto elementType = arrayType->getArrayBase();
        int32_t elementSize = static_cast<int32_t>(elementType->size());
        for (int i = 0; i < arrayType->getArraySize(); ++i) {
            emitConstantArrayStoresToAlloca(
                block,
                allocaInst,
                array->getOperand(static_cast<unsigned int>(i)),
                offset + i * elementSize);
        }
        return;
    }

    auto stackOffset = static_cast<int32_t>(
        block->getParentFunction()->getAllocaOffset(allocaInst)) + offset;
    emitScalarConstantStore(block, PhysicalRegister::get(2), constant, stackOffset);
}

InstructionTy loadTypeForValue(IR::Value* value) {
    if (value->getType()->isFloatTy()) {
        return InstructionTy::FLW;
    }
    if (value->getType()->isPointerTy()) {
        return InstructionTy::LD;
    }
    return InstructionTy::LW;
}

InstructionTy storeTypeForValue(IR::Value* value) {
    if (value->getType()->isFloatTy()) {
        return InstructionTy::FSW;
    }
    if (value->getType()->isPointerTy()) {
        return InstructionTy::SD;
    }
    return InstructionTy::SW;
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
    if (!irBlock) return successors;
    for (auto succ : irBlock->getSuccBlock()) {
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
            PhysicalRegister::get(2),
            std::make_shared<Immediate>(static_cast<int32_t>(parentFunction->getAllocaOffset(allocaInst))));
    }

    auto addr = materializeValue(this, inst->getSrc());
    auto rd = VirtualRegister::create(inst, inst->getType()->isFloatTy());
    
    return std::make_shared<IInstruction>(
        inst->getType()->isFloatTy() ? InstructionTy::FLW : InstructionTy::LW,
        rd, addr, std::make_shared<Immediate>(0));
}

std::shared_ptr<Instruction> AsmBasicBlock::convertStoreInstruction(IR::StoreInstruction* inst) {
    if (auto *constantArray = dynamic_cast<IR::ConstantArray*>(inst->getSrc())) {
        if (auto *allocaInst = dynamic_cast<IR::AllocaInstruction*>(inst->getDest())) {
            emitConstantArrayStoresToAlloca(this, allocaInst, constantArray, 0);
            return nullptr;
        }

        auto base = materializeValue(this, inst->getDest());
        emitConstantArrayStores(this, base, constantArray, 0);
        return nullptr;
    }

    if (auto *allocaInst = dynamic_cast<IR::AllocaInstruction*>(inst->getDest())) {
        auto val = materializeValue(this, inst->getSrc());
        return std::make_shared<SInstruction>(
            storeTypeForValue(inst->getSrc()),
            PhysicalRegister::get(2),
            val,
            std::make_shared<Immediate>(static_cast<int32_t>(parentFunction->getAllocaOffset(allocaInst))));
    }

    auto addr = materializeValue(this, inst->getDest());
    auto val = materializeValue(this, inst->getSrc());
    return std::make_shared<SInstruction>(
        storeTypeForValue(inst->getSrc()),
        addr, val, std::make_shared<Immediate>(0));
}

std::shared_ptr<Instruction> AsmBasicBlock::convertBinaryInstruction(IR::BinaryInstruction* inst) {
    auto lhs = materializeValue(this, inst->getOperand(0));
    auto rhs = materializeValue(this, inst->getOperand(1));
    auto rd = VirtualRegister::create(inst, inst->getType()->isFloatTy());
    
    switch (inst->getOpcode()) {
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
            
        case IR::Instruction::FAdd:
            return std::make_shared<RInstruction>(InstructionTy::FADD_S, rd, lhs, rhs);
        case IR::Instruction::FSub:
            return std::make_shared<RInstruction>(InstructionTy::FSUB_S, rd, lhs, rhs);
        case IR::Instruction::FMul:
            return std::make_shared<RInstruction>(InstructionTy::FMUL_S, rd, lhs, rhs);
        case IR::Instruction::FDiv:
            return std::make_shared<RInstruction>(InstructionTy::FDIV_S, rd, lhs, rhs);
            
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
    AnyRegister base;
    if (auto *allocaInst = dynamic_cast<IR::AllocaInstruction*>(inst->getOperand(0))) {
        base = VirtualRegister::createTemp(false);
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::ADDI,
            base,
            PhysicalRegister::get(2),
            std::make_shared<Immediate>(static_cast<int32_t>(parentFunction->getAllocaOffset(allocaInst)))));
    } else {
        base = materializeValue(this, inst->getOperand(0));
    }

    auto result = VirtualRegister::create(inst, inst->getType()->isFloatTy());
    std::vector<IR::Value*> indices = inst->getIndices();

    auto offset = VirtualRegister::createTemp(false);
    addInstruction(std::make_shared<IInstruction>(
        InstructionTy::ADDI,
        offset,
        PhysicalRegister::get(0),
        std::make_shared<Immediate>(0)));

    IR::pType indexedType = inst->getOperand(0)->getType()->getPointerBase();
    const bool pointerArgBase = dynamic_cast<IR::Argument*>(inst->getOperand(0)) != nullptr;
    for (size_t idx = 0; idx < indices.size(); ++idx) {
        auto *indexValue = indices[idx];
        if (pointerArgBase && idx == 0) {
            if (auto *zero = dynamic_cast<IR::ConstantInt32*>(indexValue)) {
                if (zero->getValue() == 0) {
                    continue;
                }
            }
        }

        auto index = materializeValue(this, indexValue);
        auto temp = VirtualRegister::createTemp(false);
        int elementBytes = static_cast<int>(indexedType->size());

        if (elementBytes == 1) {
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
                PhysicalRegister::get(0),
                std::make_shared<Immediate>(elementBytes)));
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

        indexedType = indexedType->getBase();
    }

    addInstruction(std::make_shared<RInstruction>(
        InstructionTy::ADD,
        result,
        base,
        offset));
    return nullptr;
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
                PhysicalRegister::get(0), 
                src);
                
        case IR::Instruction::FNeg:
            return std::make_shared<RInstruction>(
                InstructionTy::FSGNJN_S,  
                dst,
                src,
                src);
                
        case IR::Instruction::Not:
            return std::make_shared<IInstruction>(
                InstructionTy::XORI,
                dst,
                src,
                std::make_shared<Immediate>(-1));  
                
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
        PhysicalRegister::get(0),
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
    auto calleeName = sanitizeSymbolName(inst->getCallee()->getIRName());
    
    bool isTimingCall = calleeName == "starttime" || calleeName == "stoptime";
    if (isTimingCall) {
        auto lineNo = VirtualRegister::createTemp(false);
        addInstruction(createPseudoInstruction(
            InstructionTy::LI,
            {lineNo, std::make_shared<Immediate>(0)}));
        addInstruction(createPseudoInstruction(
            InstructionTy::MV,
            {PhysicalRegister::getParamReg(0), lineNo}));
        calleeName = calleeName == "starttime" ? "_sysy_starttime" : "_sysy_stoptime";
    }

    const int extraArgBytes = callArgs.size() > 8 ? static_cast<int>((callArgs.size() - 8) * 8) : 0;
    
    const int intSaveOffset = static_cast<int>((extraArgBytes + 7) / 8 * 8);
    const int intSaveBytes = 128;
    const int floatSaveOffset = intSaveOffset + intSaveBytes;
    const int floatSaveBytes = 64;
    const int callFrameBytes = ((floatSaveOffset + floatSaveBytes + 15) / 16) * 16;

    const size_t paramCount = std::min<size_t>(callArgs.size(), 8);
    std::vector<AnyRegister> allocatedArgs(callArgs.size());

    for (size_t i = 0; i < callArgs.size(); ++i) {
        allocatedArgs[i] = materializeValue(this, callArgs[i]);
    }

    // 🚀 焊死物理路由：彻底切断虚拟 sp 别名介入，全面采用物理寄存器 2 号进行对齐
    addInstruction(std::make_shared<IInstruction>(
        InstructionTy::ADDI, PhysicalRegister::get(2), PhysicalRegister::get(2),
        std::make_shared<Immediate>(-callFrameBytes)));

    for (size_t i = 8; i < callArgs.size(); ++i) {
        addInstruction(std::make_shared<SInstruction>(
            storeTypeForValue(callArgs[i]),
            PhysicalRegister::get(2),
            allocatedArgs[i],
            std::make_shared<Immediate>(static_cast<int32_t>((i - 8) * 8))));
    }

    for (int reg = 5; reg <= 7; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::SD, PhysicalRegister::get(2), PhysicalRegister::get(reg),
            std::make_shared<Immediate>(intSaveOffset + (reg - 5) * 8)));
    }
    for (int reg = 28; reg <= 31; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::SD, PhysicalRegister::get(2), PhysicalRegister::get(reg),
            std::make_shared<Immediate>(intSaveOffset + (reg - 25) * 8)));
    }
    for (int reg = 10; reg <= 17; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::SD, PhysicalRegister::get(2), PhysicalRegister::get(reg),
            std::make_shared<Immediate>(intSaveOffset + 64 + (reg - 10) * 8)));
    }
    for (int reg = 5; reg <= 7; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::FSW, PhysicalRegister::get(2), PhysicalRegister::get(reg, true),
            std::make_shared<Immediate>(floatSaveOffset + (reg - 5) * 4)));
    }
    for (int reg = 28; reg <= 31; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::FSW, PhysicalRegister::get(2), PhysicalRegister::get(reg, true),
            std::make_shared<Immediate>(floatSaveOffset + 12 + (reg - 28) * 4)));
    }
    for (int reg = 10; reg <= 17; ++reg) {
        addInstruction(std::make_shared<SInstruction>(
            InstructionTy::FSW, PhysicalRegister::get(2), PhysicalRegister::get(reg, true),
            std::make_shared<Immediate>(floatSaveOffset + 32 + (reg - 10) * 4)));
    }

    for (size_t i = 0; i < paramCount; ++i) {
        AnyRegister dst = PhysicalRegister::getParamReg(static_cast<int>(i), callArgs[i]->getType()->isFloatTy());
        addInstruction(createPseudoInstruction(
            callArgs[i]->getType()->isFloatTy() ? InstructionTy::FMV_S : InstructionTy::MV,
            {dst, allocatedArgs[i]}));
    }

    addInstruction(std::make_shared<Call>(
        std::make_shared<Label>(calleeName, true),
        std::vector<AnyRegister>{},
        nullptr));

    for (int reg = 5; reg <= 7; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::LD, PhysicalRegister::get(reg), PhysicalRegister::get(2),
            std::make_shared<Immediate>(intSaveOffset + (reg - 5) * 8)));
    }
    for (int reg = 28; reg <= 31; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::LD, PhysicalRegister::get(reg), PhysicalRegister::get(2),
            std::make_shared<Immediate>(intSaveOffset + (reg - 25) * 8)));
    }
    for (int reg = 10; reg <= 17; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::LD, PhysicalRegister::get(reg), PhysicalRegister::get(2),
            std::make_shared<Immediate>(intSaveOffset + 64 + (reg - 10) * 8)));
    }
    for (int reg = 5; reg <= 7; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::FLW, PhysicalRegister::get(reg, true), PhysicalRegister::get(2),
            std::make_shared<Immediate>(floatSaveOffset + (reg - 5) * 4)));
    }
    for (int reg = 28; reg <= 31; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::FLW, PhysicalRegister::get(reg, true), PhysicalRegister::get(2),
            std::make_shared<Immediate>(floatSaveOffset + 12 + (reg - 28) * 4)));
    }
    for (int reg = 10; reg <= 17; ++reg) {
        addInstruction(std::make_shared<IInstruction>(
            InstructionTy::FLW, PhysicalRegister::get(reg, true), PhysicalRegister::get(2),
            std::make_shared<Immediate>(floatSaveOffset + 32 + (reg - 10) * 4)));
    }

    addInstruction(std::make_shared<IInstruction>(
        InstructionTy::ADDI, PhysicalRegister::get(2), PhysicalRegister::get(2),
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

} // namespace backend