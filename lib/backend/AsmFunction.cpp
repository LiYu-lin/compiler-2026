#include "component.h"
#include <algorithm>
#include "OpRegister.h"
#include <memory>
#include <stack>
#include <deque>
#include <limits>
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

    std::tuple<std::unordered_set<rRegister>, std::unordered_set<rRegister>> AsmFunction::computeUseDef(const std::unique_ptr<AsmBasicBlock>& block) {
        std::unordered_set<rRegister> use_set;
        std::unordered_set<rRegister> def_set;

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

        return {std::move(use_set), std::move(def_set)};
    }

    std::tuple<std::unordered_set<rRegister>, std::unordered_set<rRegister>> AsmFunction::computeUseDefForInstruction(const std::shared_ptr<Instruction>& inst) {
        std::unordered_set<rRegister> use_set;
        std::unordered_set<rRegister> def_set;

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

        return {std::move(use_set), std::move(def_set)};
    }

    void AsmFunction::livenessAnalysis() {
        for (auto& block : blocks) {
            block->liveIn.clear();
            block->liveOut.clear();
        }

        std::vector<std::pair<std::unordered_set<rRegister>, std::unordered_set<rRegister>>> blockUseDef;
        blockUseDef.reserve(blocks.size());
        std::unordered_map<AsmBasicBlock*, size_t> blockIndex;
        for (size_t i = 0; i < blocks.size(); ++i) {
            blockIndex[blocks[i].get()] = i;
            auto pr = computeUseDef(blocks[i]);
            blockUseDef.emplace_back(std::move(std::get<0>(pr)), std::move(std::get<1>(pr)));
        }

        std::vector<std::unordered_set<rRegister>> liveInVec(blocks.size()), liveOutVec(blocks.size());
        bool changed = true;
        int safety_loop = 0;
        while (changed) {
            if (++safety_loop > 200) {
                break;
            }
            changed = false;
            for (int idx = static_cast<int>(blocks.size()) - 1; idx >= 0; --idx) {
                auto& block = blocks[idx];
                auto oldIn = liveInVec[idx];
                auto oldOut = liveOutVec[idx];

                liveOutVec[idx].clear();
                for (auto successor : block->getSuccessors()) {
                    auto sit = blockIndex.find(successor);
                    if (sit != blockIndex.end()) {
                        for (auto& r : liveInVec[sit->second]) liveOutVec[idx].insert(r);
                    }
                }

                auto& use = blockUseDef[idx].first;
                auto& def = blockUseDef[idx].second;

                liveInVec[idx] = use;
                for (auto& r : liveOutVec[idx]) {
                    if (def.find(r) == def.end()) liveInVec[idx].insert(r);
                }

                if (liveInVec[idx] != oldIn || liveOutVec[idx] != oldOut) {
                    changed = true;
                }
            }
        }

        for (size_t i = 0; i < blocks.size(); ++i) {
            blocks[i]->liveIn = std::move(liveInVec[i]);
            blocks[i]->liveOut = std::move(liveOutVec[i]);
        }

        for (auto& block : blocks) {
            std::unordered_set<rRegister> currentLive = block->liveOut;

            auto& instructions = block->getInstructions();
            for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
                auto& inst = *it;
                inst->liveOut = currentLive;

                auto [use, def] = computeUseDefForInstruction(inst);

                std::unordered_set<rRegister> newLive = std::move(use);
                for (auto& r : currentLive) {
                    if (def.find(r) == def.end()) newLive.insert(r);
                }

                currentLive = std::move(newLive);
            }

            assert(currentLive == block->liveIn);
        }
    }  

    InterferenceGraph AsmFunction::buildInterferenceGraph() {
        InterferenceGraph graph;

        std::unordered_set<rRegister> allVRegs;
        for (auto& block : blocks) {
            for (const auto& inst : block->getInstructions()) {
                for (auto& reg : inst->getDefRegisters()) {
                    if (auto v = std::dynamic_pointer_cast<VirtualRegister>(reg)) allVRegs.insert(v);
                }
                for (auto& reg : inst->getUseRegisters()) {
                    if (auto v = std::dynamic_pointer_cast<VirtualRegister>(reg)) allVRegs.insert(v);
                }
            }
        }

        graph.adjList.reserve(allVRegs.size() * 2 + 16);
        for (auto v : allVRegs) graph.adjList[v];

        for (auto& block : blocks) {
            for (const auto& inst : block->getInstructions()) {
                for (auto& reg : inst->getDefRegisters()) {
                    if (auto defVReg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                        auto liveVec = std::vector<rRegister>(inst->liveOut.begin(), inst->liveOut.end());
                        for (auto& liveReg : liveVec) {
                            graph.addEdge(defVReg, liveReg);
                        }
                    }
                }

                auto liveVec = std::vector<rRegister>(inst->liveOut.begin(), inst->liveOut.end());
                for (size_t i = 0; i < liveVec.size(); ++i) {
                    for (size_t j = i + 1; j < liveVec.size(); ++j) {
                        graph.addEdge(liveVec[i], liveVec[j]);
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

        std::unordered_map<rRegister, size_t> degree;
        std::unordered_set<rRegister> remaining;
        degree.reserve(graph.adjList.size() * 2);
        for (const auto& kv : graph.adjList) {
            degree[kv.first] = kv.second.size();
            remaining.insert(kv.first);
        }

        std::unordered_map<rRegister, size_t> useCount;
        useCount.reserve(graph.adjList.size() * 2);
        for (auto& block : blocks) {
            for (auto& inst : block->getInstructions()) {
                for (auto& reg : inst->getUseRegisters()) {
                    if (auto v = std::dynamic_pointer_cast<VirtualRegister>(reg)) useCount[v]++;
                }
            }
        }

        std::vector<rRegister> stackOrder;
        stackOrder.reserve(remaining.size());

        std::deque<rRegister> lowQueue;
        for (auto node : remaining) {
            size_t K = getAllPhysicalRegisters(node->isFloatReg()).size();
            if (degree[node] < K) lowQueue.push_back(node);
        }

        while (!remaining.empty()) {
            rRegister node = nullptr;

            if (!lowQueue.empty()) {
                node = lowQueue.front(); lowQueue.pop_front();
                if (remaining.find(node) == remaining.end()) continue;
            } else {
                double bestScore = std::numeric_limits<double>::infinity();
                for (auto n : remaining) {
                    size_t u = 1;
                    auto uit = useCount.find(n);
                    if (uit != useCount.end()) u = uit->second;
                    double score = static_cast<double>(u) / static_cast<double>(degree[n] + 1);
                    if (score < bestScore) { bestScore = score; node = n; }
                }
                if (!node) break;
            }

            remaining.erase(node);
            stackOrder.push_back(node);

            for (auto neighbor : graph.neighbors(node)) {
                if (remaining.find(neighbor) == remaining.end()) continue;
                if (degree[neighbor] > 0) degree[neighbor]--;
                size_t K = getAllPhysicalRegisters(neighbor->isFloatReg()).size();
                if (degree[neighbor] < K) lowQueue.push_back(neighbor);
            }
        }

        for (auto it = stackOrder.rbegin(); it != stackOrder.rend(); ++it) {
            auto node = *it;
            std::unordered_set<pRegister> usedColors;
            for (auto neighbor : graph.neighbors(node)) {
                auto cit = coloring.find(neighbor);
                if (cit != coloring.end()) usedColors.insert(cit->second);
            }

            bool assigned = false;
            for (auto reg : getAllPhysicalRegisters(node->isFloatReg())) {
                if (usedColors.find(reg) == usedColors.end()) {
                    coloring[node] = reg;
                    assigned = true;
                    break;
                }
            }
            if (!assigned) {
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