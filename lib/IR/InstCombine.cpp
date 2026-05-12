#include "InstCombine.h"
#include "constant.h"
#include "instruction.h"

namespace IR {

namespace {

bool isInt(Value* value, int expected) {
    return value && value->isConstantInt32() &&
           static_cast<ConstantInt32*>(value)->getValue() == expected;
}

bool isFloat(Value* value, float expected) {
    return value && value->isConstantFloat() &&
           static_cast<ConstantFloat*>(value)->getValue() == expected;
}

Value* foldIntBinary(BinaryInstruction* inst, int lhs, int rhs) {
    switch (inst->getOpcode()) {
        case Instruction::Add:
            return ConstantInt32::get(lhs + rhs);
        case Instruction::Sub:
            return ConstantInt32::get(lhs - rhs);
        case Instruction::Mul:
            return ConstantInt32::get(lhs * rhs);
        case Instruction::Div:
            return rhs == 0 ? nullptr : ConstantInt32::get(lhs / rhs);
        case Instruction::Rem:
            return rhs == 0 ? nullptr : ConstantInt32::get(lhs % rhs);
        case Instruction::And:
            return ConstantInt32::get(lhs & rhs);
        case Instruction::Or:
            return ConstantInt32::get(lhs | rhs);
        case Instruction::Xor:
            return ConstantInt32::get(lhs ^ rhs);
        default:
            return nullptr;
    }
}

Value* foldFloatBinary(BinaryInstruction* inst, float lhs, float rhs) {
    switch (inst->getOpcode()) {
        case Instruction::FAdd:
            return ConstantFloat::get(lhs + rhs);
        case Instruction::FSub:
            return ConstantFloat::get(lhs - rhs);
        case Instruction::FMul:
            return ConstantFloat::get(lhs * rhs);
        case Instruction::FDiv:
            return rhs == 0.0f ? nullptr : ConstantFloat::get(lhs / rhs);
        default:
            return nullptr;
    }
}

Value* simplifyBinary(BinaryInstruction* inst) {
    auto* lhs = inst->getOperand(0);
    auto* rhs = inst->getOperand(1);

    if (lhs->isConstantInt32() && rhs->isConstantInt32()) {
        return foldIntBinary(inst,
                             static_cast<ConstantInt32*>(lhs)->getValue(),
                             static_cast<ConstantInt32*>(rhs)->getValue());
    }

    if (lhs->isConstantFloat() && rhs->isConstantFloat()) {
        return foldFloatBinary(inst,
                               static_cast<ConstantFloat*>(lhs)->getValue(),
                               static_cast<ConstantFloat*>(rhs)->getValue());
    }

    switch (inst->getOpcode()) {
        case Instruction::Add:
            if (isInt(rhs, 0)) return lhs;
            if (isInt(lhs, 0)) return rhs;
            break;
        case Instruction::Sub:
            if (isInt(rhs, 0)) return lhs;
            if (lhs == rhs) return ConstantInt32::get(0);
            break;
        case Instruction::Mul:
            if (isInt(rhs, 1)) return lhs;
            if (isInt(lhs, 1)) return rhs;
            if (isInt(rhs, 0) || isInt(lhs, 0)) return ConstantInt32::get(0);
            break;
        case Instruction::Div:
            if (isInt(rhs, 1)) return lhs;
            if (isInt(lhs, 0)) return ConstantInt32::get(0);
            break;
        case Instruction::Rem:
            if (isInt(lhs, 0) || isInt(rhs, 1)) return ConstantInt32::get(0);
            break;
        case Instruction::And:
            if (isInt(rhs, 0) || isInt(lhs, 0)) return ConstantInt32::get(0);
            if (isInt(rhs, -1)) return lhs;
            if (isInt(lhs, -1)) return rhs;
            if (lhs == rhs) return lhs;
            break;
        case Instruction::Or:
            if (isInt(rhs, 0)) return lhs;
            if (isInt(lhs, 0)) return rhs;
            if (lhs == rhs) return lhs;
            break;
        case Instruction::Xor:
            if (isInt(rhs, 0)) return lhs;
            if (isInt(lhs, 0)) return rhs;
            if (lhs == rhs) return ConstantInt32::get(0);
            break;
        case Instruction::FAdd:
            if (isFloat(rhs, 0.0f)) return lhs;
            if (isFloat(lhs, 0.0f)) return rhs;
            break;
        case Instruction::FSub:
            if (isFloat(rhs, 0.0f)) return lhs;
            break;
        case Instruction::FMul:
            if (isFloat(rhs, 1.0f)) return lhs;
            if (isFloat(lhs, 1.0f)) return rhs;
            if (isFloat(rhs, 0.0f) || isFloat(lhs, 0.0f)) return ConstantFloat::get(0.0f);
            break;
        case Instruction::FDiv:
            if (isFloat(rhs, 1.0f)) return lhs;
            break;
        default:
            break;
    }

    return nullptr;
}

Value* simplifyCmp(CmpInstruction* inst) {
    auto* lhs = inst->getOperand(0);
    auto* rhs = inst->getOperand(1);

    if (lhs->isConstantInt32() && rhs->isConstantInt32()) {
        int l = static_cast<ConstantInt32*>(lhs)->getValue();
        int r = static_cast<ConstantInt32*>(rhs)->getValue();
        switch (inst->getCmpCode()) {
            case Instruction::Eq: return ConstantInt32::get(l == r);
            case Instruction::Ne: return ConstantInt32::get(l != r);
            case Instruction::Lt: return ConstantInt32::get(l < r);
            case Instruction::Le: return ConstantInt32::get(l <= r);
            case Instruction::Gt: return ConstantInt32::get(l > r);
            case Instruction::Ge: return ConstantInt32::get(l >= r);
            default: break;
        }
    }

    if (lhs->isConstantFloat() && rhs->isConstantFloat()) {
        float l = static_cast<ConstantFloat*>(lhs)->getValue();
        float r = static_cast<ConstantFloat*>(rhs)->getValue();
        switch (inst->getCmpCode()) {
            case Instruction::FEq: return ConstantInt32::get(l == r);
            case Instruction::FNe: return ConstantInt32::get(l != r);
            case Instruction::FLt: return ConstantInt32::get(l < r);
            case Instruction::FLe: return ConstantInt32::get(l <= r);
            case Instruction::FGt: return ConstantInt32::get(l > r);
            case Instruction::FGe: return ConstantInt32::get(l >= r);
            default: break;
        }
    }

    if (lhs == rhs) {
        switch (inst->getCmpCode()) {
            case Instruction::Eq:
            case Instruction::Le:
            case Instruction::Ge:
                return ConstantInt32::get(1);
            case Instruction::Ne:
            case Instruction::Lt:
            case Instruction::Gt:
                return ConstantInt32::get(0);
            default:
                break;
        }
    }

    return nullptr;
}

Value* simplifyUnary(UnaryInstruction* inst) {
    auto* operand = inst->getOperand(0);
    switch (inst->getOpcode()) {
        case Instruction::Neg:
            if (operand->isConstantInt32()) {
                return ConstantInt32::get(-static_cast<ConstantInt32*>(operand)->getValue());
            }
            break;
        case Instruction::FNeg:
            if (operand->isConstantFloat()) {
                return ConstantFloat::get(-static_cast<ConstantFloat*>(operand)->getValue());
            }
            break;
        case Instruction::Not:
            if (operand->isConstantInt32()) {
                return ConstantInt32::get(!static_cast<ConstantInt32*>(operand)->getValue());
            }
            break;
        default:
            break;
    }
    return nullptr;
}

Value* simplifyCast(CastInstruction* inst) {
    auto* operand = inst->getOperand(0);
    switch (inst->getOpcode()) {
        case Instruction::FPtoSI:
            if (operand->isConstantFloat()) {
                return ConstantInt32::get(static_cast<int>(static_cast<ConstantFloat*>(operand)->getValue()));
            }
            break;
        case Instruction::SItoFP:
            if (operand->isConstantInt32()) {
                return ConstantFloat::get(static_cast<float>(static_cast<ConstantInt32*>(operand)->getValue()));
            }
            break;
        default:
            break;
    }
    return nullptr;
}

}  // namespace

bool InstCombine::run() {
    foldedInstructions = 0;
    bool changed = false;

    for (auto* function : module.getFunctionList()) {
        if (!function || function->isBuiltinFunction()) {
            continue;
        }

        for (auto* block : function->getVectorBlocks()) {
            auto instructions = block->getVectorInstructions();
            for (auto* instruction : instructions) {
                if (!instruction || instruction->useList.empty()) {
                    continue;
                }

                Value* replacement = nullptr;
                auto opcode = instruction->getOpcode();
                if (opcode >= Instruction::BinaryBegin && opcode < Instruction::BinaryEnd) {
                    replacement = simplifyBinary(static_cast<BinaryInstruction*>(instruction));
                } else if (opcode >= Instruction::CmpBegin && opcode < Instruction::CmpEnd) {
                    replacement = simplifyCmp(static_cast<CmpInstruction*>(instruction));
                } else if (opcode >= Instruction::UnaryBegin && opcode < Instruction::UnaryEnd) {
                    replacement = simplifyUnary(static_cast<UnaryInstruction*>(instruction));
                } else if (opcode >= Instruction::CastBegin && opcode < Instruction::CastEnd) {
                    replacement = simplifyCast(static_cast<CastInstruction*>(instruction));
                }

                if (!replacement || replacement == instruction) {
                    continue;
                }

                instruction->replaceAllUsageTo(replacement);
                instruction->waste();
                ++foldedInstructions;
                changed = true;
            }
        }
    }

    return changed;
}

std::map<std::string, int> InstCombine::stats() const {
    return {{"folded_instructions", foldedInstructions}};
}

}  // namespace IR
