#include "component.h"
#include <algorithm>
#include "OpRegister.h"
#include <memory>
#include <stack>

namespace backend {
    static std::vector<pRegister> physicalRegs;
    std::unordered_map<rRegister, size_t> spillOffsets;

    namespace {
        void replaceRegisterOperand(const std::shared_ptr<Instruction>& inst, AnyRegister oldReg, AnyRegister newReg) {
            for (int i = 0; i < 4; ++i) {
                try {
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

        // 遍历块内的每一条机器指??
        for (auto& instr : block->getInstructions()) {
            // 首先处理所有源操作??USE)
            for (auto& reg : instr->getUseRegisters()) {
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                    // 如果这个寄存器还没有被定义过(不在def_set??
                    // 那么它就是在使用一个来自块外的值，属于USE集合
                    if (def_set.find(vreg) == def_set.end()) {
                        use_set.insert(vreg);
                    }
                }
            }

            // 然后处理所有目标操作数(DEF)
            for (auto& reg : instr->getDefRegisters()) {
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                    // 只要被写入，就加入DEF集合
                    def_set.insert(vreg);
                }
            }
        }

        return {use_set, def_set};
    }
    std::tuple<std::set<rRegister>, std::set<rRegister>> AsmFunction::computeUseDefForInstruction(const std::shared_ptr<Instruction>& inst) {
        std::set<rRegister> use_set;
        std::set<rRegister> def_set;

        // 处理指令的源操作??USE)
        for (auto& reg : inst->getUseRegisters()) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                // 如果寄存器不在DEF集合中，则加入USE集合
                if (def_set.find(vreg) == def_set.end()) {
                    use_set.insert(vreg);
                }
            }
        }

        // 处理指令的目标操作数(DEF)
        for (auto& reg : inst->getDefRegisters()) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                def_set.insert(vreg);
            }
        }

        return {use_set, def_set};
    }

    void AsmFunction::livenessAnalysis() {
        // 初始??
        for (auto& block : blocks) {
            block->liveIn.clear();
            block->liveOut.clear();
        }

        bool changed = true;
        while (changed) {
            changed = false;
            
            // 反向遍历所有基本块
            for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
                auto& block = *it;
                
                // 保存旧值用于比??
                std::set<rRegister> oldIn = block->liveIn;
                std::set<rRegister> oldOut = block->liveOut;
                
                // 计算新的OUT[B] = ??IN[S] (对于所有后继S)
                block->liveOut.clear();
                for (auto successor : block->getSuccessors()) {
                    block->liveOut.insert(successor->liveIn.begin(), successor->liveIn.end());
                }
                
                // 计算新的IN[B] = USE[B] ??(OUT[B] - DEF[B])
                auto [use, def] = computeUseDef(block); // 需要实现这个函??
                
                block->liveIn = use;
                std::set<rRegister> temp;
                std::set_difference(block->liveOut.begin(), block->liveOut.end(),
                                def.begin(), def.end(),
                                std::inserter(temp, temp.begin()));
                block->liveIn.insert(temp.begin(), temp.end());
                
                // 检查是否变??
                if (block->liveIn != oldIn || block->liveOut != oldOut) {
                    changed = true;
                }
            }
            for (auto& block : blocks) {
                std::set<rRegister> currentLive = block->liveOut;
                
                // 反向遍历块内的指??
                auto& instructions = block->getInstructions();
                for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
                    auto& inst = *it;
                    
                    // 1. 设置当前指令的liveOut
                    inst->liveOut = currentLive;
                    
                    // 2. 计算当前指令的USE和DEF
                    auto [use, def] = computeUseDefForInstruction(inst);
                    
                    // 3. 计算前一条指令的活性：IN = USE ??(OUT - DEF)
                    std::set<rRegister> newLive = use;
                    std::set<rRegister> temp;
                    std::set_difference(currentLive.begin(), currentLive.end(),
                                    def.begin(), def.end(),
                                    std::inserter(temp, temp.begin()));
                    newLive.insert(temp.begin(), temp.end());
                    
                    // 4. 为下一条指令准??
                    currentLive = newLive;
                }
                
                // 验证：currentLive应该等于block->liveIn
                assert(currentLive == block->liveIn);
            }
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
                const auto& live = inst->liveOut; // 你需要在活跃变量分析时填??
                for (auto it1 = live.begin(); it1 != live.end(); ++it1) {
                    for (auto it2 = std::next(it1); it2 != live.end(); ++it2) {
                        graph.addEdge(*it1, *it2);
                    }
                }
            }
        }
        return graph;
    }
    
    std::vector<pRegister>& getAllPhysicalRegisters(bool isFloat = false) {
        static std::vector<pRegister> intRegs;
        static std::vector<pRegister> floatRegs;
        auto& regs = isFloat ? floatRegs : intRegs;
        if (regs.empty()) {
            const int allocatableRegs[] = {
                5, 6, 7,
                28, 29, 30, 31
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
        
        // 1. 图简化阶??
        auto workGraph = graph;
        while (!workGraph.nodes().empty()) {
            bool simplified = false;
            
            // 尝试找到可简化的节点
            for (auto node : workGraph.nodes()) {
                if (workGraph.neighbors(node).size() < getAllPhysicalRegisters(node->isFloatReg()).size()) {
                    stack.push(node);
                    workGraph.removeNode(node);
                    simplified = true;
                    break;
                }
            }
            
            // 如果没有可简化的节点，选择溢出
            if (!simplified) {
                auto node = workGraph.getMaxDegreeNode();
                stack.push(node);
                workGraph.removeNode(node);
            }
        }
        
        // 2. 着色阶??
        while (!stack.empty()) {
            auto node = stack.top();
            stack.pop();
            
            std::set<pRegister> usedColors;
            for (auto neighbor : graph.neighbors(node)) {
                if (coloring.count(neighbor)) {
                    usedColors.insert(coloring[neighbor]);
                }
            }
            
            // 找到可用的颜??
            for (auto reg : getAllPhysicalRegisters(node->isFloatReg())) {
                if (usedColors.count(reg) == 0) {
                    coloring[node] = reg;
                    break;
                }
            }
            
            // 如果没有可用颜色，标记为溢出
            if (coloring.count(node) == 0) {
                markForSpilling(node);
            }
        }
        
        return coloring;
    }

    void AsmFunction::rewriteInstructions(std::unordered_map<rRegister, pRegister>& vregToPreg) {
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
            if (vregToPreg.count(vreg) || spilledNodes.count(vreg)) {
                continue;
            }
            markForSpilling(vreg);
        }
        // 1. 首先处理所有已分配的虚拟寄存器
        for (auto& block : blocks) {
            for (auto& inst : block->getInstructions()) {
                inst->replaceVRegsWithPhysRegs(vregToPreg);
            }
        }
        
        // 2. 处理需要溢出的寄存??
        for (const auto& spilledReg : spilledNodes) {
            size_t spillOffset = reserveSpillSlot();
            spillOffsets[spilledReg] = spillOffset;
            // 在所有使用该寄存器的地方插入加载/存储指令
            for (auto& block : blocks) {
                auto& instructions = block->getInstructions();
                
                std::vector<std::shared_ptr<Instruction>> instructionsToProcess;
                for (auto& inst : instructions) {
                    instructionsToProcess.push_back(inst);
                }
                
                for (auto& inst : instructionsToProcess) {
                    // 检查指令是否使用溢出的寄存??
                    bool usesSpilledReg = false;
                    for (auto& reg : inst->getUseRegisters()) {
                        if (reg == spilledReg) {
                            usesSpilledReg = true;
                            break;
                        }
                    }
                    for (auto& reg : inst->getDefRegisters()) {
                        if (reg == spilledReg) {
                            usesSpilledReg = true;
                            break;
                        }
                    }
                    
                    if (usesSpilledReg) {
                        // 在指令前插入加载指令（如果使用该寄存器）
                        for (auto& reg : inst->getUseRegisters()) {
                            if (reg == spilledReg) {
                                AnyRegister tempReg = PhysicalRegister::get(5, spilledReg->isFloatReg());
                                // 根据寄存器类型选择正确的指令类??
                                InstructionTy loadTy = spilledReg->isFloatReg() ? InstructionTy::FLW : InstructionTy::LW;
                                auto loadInst = std::make_shared<IInstruction>(
                                    loadTy,
                                    tempReg, 
                                    PhysicalRegister::get(2), 
                                    std::make_shared<Immediate>(spillOffset)
                                );
                                block->insertBefore(inst, loadInst);
                                inst->replaceRegisterUse(spilledReg, tempReg);
                                replaceRegisterOperand(inst, spilledReg, tempReg);
                            }
                        }
                        
                        // 在指令后插入存储指令（如果定义该寄存器）
                        for (auto& reg : inst->getDefRegisters()) {
                            if (reg == spilledReg) {
                                AnyRegister tempReg = PhysicalRegister::get(5, spilledReg->isFloatReg());
                                // 根据寄存器类型选择正确的指令类??
                                InstructionTy storeTy = spilledReg->isFloatReg() ? InstructionTy::FSW : InstructionTy::SW;
                                auto storeInst = std::make_shared<SInstruction>(
                                    storeTy,
                                    PhysicalRegister::get(2),
                                    tempReg,
                                    std::make_shared<Immediate>(spillOffset)
                                );
                                block->insertAfter(inst, storeInst);
                                inst->replaceRegisterDef(spilledReg, tempReg);
                                replaceRegisterOperand(inst, spilledReg, tempReg);
                            }
                        }
                    }
                }
            }
        }
        for (auto& block : blocks) {
            for (auto& inst : block->getInstructions()) {
                for (auto& reg : inst->getDefRegisters()) {
                    if (std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                        std::cerr << "Warning: Unmapped virtual register in def: " << reg->toString() << std::endl;
                    }
                }
                for (auto& reg : inst->getUseRegisters()) {
                    if (std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                        std::cerr << "Warning: Unmapped virtual register in use: " << reg->toString() << std::endl;
                    }
                }
            }
        }

    }
}


