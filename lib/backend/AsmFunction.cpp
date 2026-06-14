#include "component.h"
#include <algorithm>
#include "OpRegister.h"
#include <memory>
#include <stack>
#include <iostream>
#include <cassert>

namespace backend {
    static std::vector<pRegister> physicalRegs;
    std::unordered_map<rRegister, size_t> spillOffsets;

    namespace {
        bool operandIndexIsUse(InstType type, int index) {
            switch (type) {
                case InstType::R:
                    return index == 1 || index == 2;
                case InstType::I:
                    return index == 1;
                case InstType::S:
                case InstType::B:
                    return index == 0 || index == 1;
                case InstType::J:
                case InstType::U:
                    return false;
                case InstType::Pseudo:
                    return index > 0;
            }
            return false;
        }

        void replaceRegisterOperand(const std::shared_ptr<Instruction>& inst, AnyRegister oldReg, AnyRegister newReg, bool replaceDefs = true) {
            for (int i = 0; i < 4; ++i) {
                try {
                    if (!replaceDefs && !operandIndexIsUse(inst->getInstType(), i)) {
                        continue;
                    }
                    auto operand = inst->getOperand(i);
                    if (std::holds_alternative<AnyRegister>(operand) && std::get<AnyRegister>(operand) == oldReg) {
                        inst->setOperand(i, newReg);
                    }
                } catch (...) {
                    break;
                }
            }
        }

        pRegister getHintedPhysicalRegister(const rRegister& reg) {
            if (!reg) {
                return nullptr;
            }

            if (auto *arg = dynamic_cast<IR::Argument*>(reg->getIRValue())) {
                if (arg->getArgNo() >= 0 && arg->getArgNo() < 8) {
                    return PhysicalRegister::getParamReg(arg->getArgNo(), reg->isFloatReg());
                }
            }

            const auto& hint = reg->getHint();
            if (hint == "x0") return PhysicalRegister::get(0);
            if (hint == "sp") return PhysicalRegister::get(2);
            if (hint == "a0") return PhysicalRegister::get(10);
            if (hint == "f0") return PhysicalRegister::get(0, true);
            if (hint == "fa0") return PhysicalRegister::get(10, true);
            return nullptr;
        }
    }

    std::tuple<std::set<rRegister>, std::set<rRegister>> AsmFunction::computeUseDef(const std::unique_ptr<AsmBasicBlock>& block) {
        std::set<rRegister> use_set;
        std::set<rRegister> def_set;

        for (auto& instr : block->getInstructions()) {
            for (auto& reg : instr->getUseRegisters()) {
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                    if (def_set.find(vreg) == def_set.end()) {
                        use_set.insert(vreg);
                    }
                }
            }

            for (auto& reg : instr->getDefRegisters()) {
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                    def_set.insert(vreg);
                }
            }
        }

        return {use_set, def_set};
    }

    std::tuple<std::set<rRegister>, std::set<rRegister>> AsmFunction::computeUseDefForInstruction(const std::shared_ptr<Instruction>& inst) {
        std::set<rRegister> use_set;
        std::set<rRegister> def_set;

        for (auto& reg : inst->getUseRegisters()) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                if (def_set.find(vreg) == def_set.end()) {
                    use_set.insert(vreg);
                }
            }
        }

        for (auto& reg : inst->getDefRegisters()) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                def_set.insert(vreg);
            }
        }

        return {use_set, def_set};
    }

    void AsmFunction::livenessAnalysis() {
        for (auto& block : blocks) {
            block->liveIn.clear();
            block->liveOut.clear();
        }

        bool changed = true;
        while (changed) {
            changed = false;
            
            for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
                auto& block = *it;
                
                std::set<rRegister> oldIn = block->liveIn;
                std::set<rRegister> oldOut = block->liveOut;
                
                block->liveOut.clear();
                for (auto successor : block->getSuccessors()) {
                    block->liveOut.insert(successor->liveIn.begin(), successor->liveIn.end());
                }
                
                auto [use, def] = computeUseDef(block); 
                
                block->liveIn = use;
                std::set<rRegister> temp;
                std::set_difference(block->liveOut.begin(), block->liveOut.end(),
                                    def.begin(), def.end(),
                                    std::inserter(temp, temp.begin()));
                block->liveIn.insert(temp.begin(), temp.end());
                
                if (block->liveIn != oldIn || block->liveOut != oldOut) {
                    changed = true;
                }
            }
        }

        for (auto& block : blocks) {
            std::set<rRegister> currentLive = block->liveOut;
            
            auto& instructions = block->getInstructions();
            for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
                auto& inst = *it;
                
                inst->liveOut = currentLive;
                
                auto [use, def] = computeUseDefForInstruction(inst);
                
                std::set<rRegister> newLive = use;
                std::set<rRegister> temp;
                std::set_difference(currentLive.begin(), currentLive.end(),
                                    def.begin(), def.end(),
                                    std::inserter(temp, temp.begin()));
                newLive.insert(temp.begin(), temp.end());
                
                currentLive = newLive;
            }
            
            assert(currentLive == block->liveIn);
        }
    }  

    InterferenceGraph AsmFunction::buildInterferenceGraph() {
        InterferenceGraph graph;
        for (auto& block : blocks) {
            for (const auto& inst : block->getInstructions()) {
                for (auto& reg : inst->getDefRegisters()) {
                    if (auto defVReg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                        graph.adjList[defVReg];
                        for (auto& liveReg : inst->liveOut) {
                            graph.addEdge(defVReg, liveReg);
                        }
                    }
                }
                const auto& live = inst->liveOut; 
                for (auto it1 = live.begin(); it1 != live.end(); ++it1) {
                    for (auto it2 = std::next(it1); it2 != live.end(); ++it2) {
                        graph.addEdge(*it1, *it2);
                    }
                }
            }
        }
        return graph;
    }
    
    std::vector<pRegister>& getAllPhysicalRegisters(bool isFloat) {
        static std::vector<pRegister> intRegs;
        static std::vector<pRegister> floatRegs;
        auto& regs = isFloat ? floatRegs : intRegs;
        if (regs.empty()) {
            const int allocatableRegs[] = {
                5, 6, 7,
                28, 29, 30
            };
            for (int reg : allocatableRegs) {
                regs.push_back(PhysicalRegister::get(reg, isFloat));
            }
        }
        return regs;
    }

    std::unordered_map<rRegister, pRegister> AsmFunction::colorGraph(const InterferenceGraph& graph) {
        std::unordered_map<rRegister, pRegister> coloring;
        std::stack<rRegister> stack;
        
        auto workGraph = graph;
        while (!workGraph.nodes().empty()) {
            bool simplified = false;
            
            for (auto node : workGraph.nodes()) {
                if (workGraph.neighbors(node).size() < getAllPhysicalRegisters(node->isFloatReg()).size()) {
                    stack.push(node);
                    workGraph.removeNode(node);
                    simplified = true;
                    break;
                }
            }
            
            if (!simplified) {
                auto node = workGraph.getMaxDegreeNode();
                if (!node) break;
                stack.push(node);
                workGraph.removeNode(node);
            }
        }
        
        while (!stack.empty()) {
            auto node = stack.top();
            stack.pop();
            
            std::set<pRegister> usedColors;
            for (auto neighbor : graph.neighbors(node)) {
                if (coloring.count(neighbor)) {
                    usedColors.insert(coloring[neighbor]);
                }
            }
            
            for (auto reg : getAllPhysicalRegisters(node->isFloatReg())) {
                if (usedColors.count(reg) == 0) {
                    coloring[node] = reg;
                    break;
                }
            }
            
            if (coloring.count(node) == 0) {
                markForSpilling(node);
            }
        }
        
        return coloring;
    }

    void AsmFunction::rewriteInstructions(std::unordered_map<rRegister, pRegister>& vregToPreg) {
        if (vregToPreg.empty() && !spilledNodes.empty()) {
            for (const auto& spilledReg : spilledNodes) {
                if (spillOffsets.count(spilledReg)) continue;
                size_t spillOffset = reserveSpillSlot();
                spillOffsets[spilledReg] = spillOffset;
            }

            for (auto& block : blocks) {
                auto& instructions = block->getInstructions();
                std::vector<std::shared_ptr<Instruction>> instructionsToProcess = instructions;
                
                for (auto& inst : instructionsToProcess) {
                    for (auto& reg : inst->getUseRegisters()) {
                        if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                            if (spilledNodes.count(vreg)) {
                                bool isFloat = vreg->isFloatReg();
                                bool isPtr = (vreg->getIRValue() && vreg->getIRValue()->getType()->isPointerTy());
                                auto tempVReg = VirtualRegister::createTemp(isFloat);
                                InstructionTy loadTy = isFloat ? InstructionTy::FLW : (isPtr ? InstructionTy::LD : InstructionTy::LW);
                                
                                auto loadInst = std::make_shared<IInstruction>(
                                    loadTy,
                                    tempVReg,
                                    PhysicalRegister::get(2),
                                    std::make_shared<Immediate>(static_cast<int32_t>(spillOffsets[vreg]))
                                );
                                block->insertBefore(inst, loadInst);
                                inst->replaceRegisterUse(vreg, tempVReg);
                                replaceRegisterOperand(inst, vreg, tempVReg, false);
                            }
                        }
                    }
                    
                    for (auto& reg : inst->getDefRegisters()) {
                        if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                            if (spilledNodes.count(vreg)) {
                                bool isFloat = vreg->isFloatReg();
                                bool isPtr = (vreg->getIRValue() && vreg->getIRValue()->getType()->isPointerTy());
                                auto tempVReg = VirtualRegister::createTemp(isFloat);
                                InstructionTy storeTy = isFloat ? InstructionTy::FSW : (isPtr ? InstructionTy::SD : InstructionTy::SW);
                                
                                auto storeInst = std::make_shared<SInstruction>(
                                    storeTy,
                                    PhysicalRegister::get(2),
                                    tempVReg,
                                    std::make_shared<Immediate>(static_cast<int32_t>(spillOffsets[vreg]))
                                );
                                block->insertAfter(inst, storeInst);
                                inst->replaceRegisterDef(vreg, tempVReg);
                                replaceRegisterOperand(inst, vreg, tempVReg);
                            }
                        }
                    }
                }
            }
            return;
        }

        std::set<rRegister> allVRegs;
        for (auto& block : blocks) {
            for (auto& inst : block->getInstructions()) {
                for (auto& reg : inst->getDefRegisters()) {
                    if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg))
                        allVRegs.insert(vreg);
                }
                for (auto& reg : inst->getUseRegisters()) {
                    if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg))
                        allVRegs.insert(vreg);
                }
            }
        }
        for (auto& vreg : allVRegs) {
            if (auto hinted = getHintedPhysicalRegister(vreg)) {
                vregToPreg[vreg] = hinted;
                continue;
            }
        }
        
        for (auto& block : blocks) {
            for (auto& inst : block->getInstructions()) {
                inst->replaceVRegsWithPhysRegs(vregToPreg);
            }
        }
    }
}