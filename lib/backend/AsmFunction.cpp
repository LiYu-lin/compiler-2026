#include "backend/component.h"
#include <algorithm>
#include "backend/RiscVOperand/OpRegister.h"
#include <memory>
#include <stack>

namespace backend {
    static std::vector<pRegister> physicalRegs;
    std::unordered_map<rRegister, size_t> spillOffsets;

    namespace {
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

        // 閬嶅巻鍧楀唴鐨勬瘡涓€鏉℃満鍣ㄦ寚浠?
        for (auto& instr : block->getInstructions()) {
            // 棣栧厛澶勭悊鎵€鏈夋簮鎿嶄綔鏁?USE)
            for (auto& reg : instr->getUseRegisters()) {
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                    // 濡傛灉杩欎釜瀵勫瓨鍣ㄨ繕娌℃湁琚畾涔夎繃(涓嶅湪def_set涓?
                    // 閭ｄ箞瀹冨氨鏄湪浣跨敤涓€涓潵鑷潡澶栫殑鍊硷紝灞炰簬USE闆嗗悎
                    if (def_set.find(vreg) == def_set.end()) {
                        use_set.insert(vreg);
                    }
                }
            }

            // 鐒跺悗澶勭悊鎵€鏈夌洰鏍囨搷浣滄暟(DEF)
            for (auto& reg : instr->getDefRegisters()) {
                if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                    // 鍙琚啓鍏ワ紝灏卞姞鍏EF闆嗗悎
                    def_set.insert(vreg);
                }
            }
        }

        return {use_set, def_set};
    }
    std::tuple<std::set<rRegister>, std::set<rRegister>> AsmFunction::computeUseDefForInstruction(const std::shared_ptr<Instruction>& inst) {
        std::set<rRegister> use_set;
        std::set<rRegister> def_set;

        // 澶勭悊鎸囦护鐨勬簮鎿嶄綔鏁?USE)
        for (auto& reg : inst->getUseRegisters()) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                // 濡傛灉瀵勫瓨鍣ㄤ笉鍦―EF闆嗗悎涓紝鍒欏姞鍏SE闆嗗悎
                if (def_set.find(vreg) == def_set.end()) {
                    use_set.insert(vreg);
                }
            }
        }

        // 澶勭悊鎸囦护鐨勭洰鏍囨搷浣滄暟(DEF)
        for (auto& reg : inst->getDefRegisters()) {
            if (auto vreg = std::dynamic_pointer_cast<VirtualRegister>(reg)) {
                def_set.insert(vreg);
            }
        }

        return {use_set, def_set};
    }

    void AsmFunction::livenessAnalysis() {
        // 鍒濆鍖?
        for (auto& block : blocks) {
            block->liveIn.clear();
            block->liveOut.clear();
        }

        bool changed = true;
        while (changed) {
            changed = false;
            
            // 鍙嶅悜閬嶅巻鎵€鏈夊熀鏈潡
            for (auto it = blocks.rbegin(); it != blocks.rend(); ++it) {
                auto& block = *it;
                
                // 淇濆瓨鏃у€肩敤浜庢瘮杈?
                std::set<rRegister> oldIn = block->liveIn;
                std::set<rRegister> oldOut = block->liveOut;
                
                // 璁＄畻鏂扮殑OUT[B] = 鈭?IN[S] (瀵逛簬鎵€鏈夊悗缁)
                block->liveOut.clear();
                for (auto successor : block->getSuccessors()) {
                    block->liveOut.insert(successor->liveIn.begin(), successor->liveIn.end());
                }
                
                // 璁＄畻鏂扮殑IN[B] = USE[B] 鈭?(OUT[B] - DEF[B])
                auto [use, def] = computeUseDef(block); // 闇€瑕佸疄鐜拌繖涓嚱鏁?
                
                block->liveIn = use;
                std::set<rRegister> temp;
                std::set_difference(block->liveOut.begin(), block->liveOut.end(),
                                def.begin(), def.end(),
                                std::inserter(temp, temp.begin()));
                block->liveIn.insert(temp.begin(), temp.end());
                
                // 妫€鏌ユ槸鍚﹀彉鍖?
                if (block->liveIn != oldIn || block->liveOut != oldOut) {
                    changed = true;
                }
            }
            for (auto& block : blocks) {
                std::set<rRegister> currentLive = block->liveOut;
                
                // 鍙嶅悜閬嶅巻鍧楀唴鐨勬寚浠?
                auto& instructions = block->getInstructions();
                for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
                    auto& inst = *it;
                    
                    // 1. 璁剧疆褰撳墠鎸囦护鐨刲iveOut
                    inst->liveOut = currentLive;
                    
                    // 2. 璁＄畻褰撳墠鎸囦护鐨刄SE鍜孌EF
                    auto [use, def] = computeUseDefForInstruction(inst);
                    
                    // 3. 璁＄畻鍓嶄竴鏉℃寚浠ょ殑娲绘€э細IN = USE 鈭?(OUT - DEF)
                    std::set<rRegister> newLive = use;
                    std::set<rRegister> temp;
                    std::set_difference(currentLive.begin(), currentLive.end(),
                                    def.begin(), def.end(),
                                    std::inserter(temp, temp.begin()));
                    newLive.insert(temp.begin(), temp.end());
                    
                    // 4. 涓轰笅涓€鏉℃寚浠ゅ噯澶?
                    currentLive = newLive;
                }
                
                // 楠岃瘉锛歝urrentLive搴旇绛変簬block->liveIn
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
                const auto& live = inst->liveOut; // 浣犻渶瑕佸湪娲昏穬鍙橀噺鍒嗘瀽鏃跺～鍏?
                for (auto it1 = live.begin(); it1 != live.end(); ++it1) {
                    for (auto it2 = std::next(it1); it2 != live.end(); ++it2) {
                        graph.addEdge(*it1, *it2);
                    }
                }
            }
        }
        return graph;
    }
    
    std::vector<pRegister>& getAllPhysicalRegisters() {
        if (physicalRegs.empty()) {
            const int allocatableRegs[] = {
                5, 6, 7,       // t0-t2
                28, 29, 30, 31 // t3-t6
            };
            for (int reg : allocatableRegs) {
                physicalRegs.push_back(PhysicalRegister::get(reg));
            }
        }
        return physicalRegs;
        //杩樻湭娣诲姞娴偣瀵勫瓨鍣?
    }


    std::unordered_map<rRegister, pRegister> AsmFunction::colorGraph(const InterferenceGraph& graph) {
        std::unordered_map<rRegister, pRegister> coloring;
        std::stack<rRegister> stack;
        
        // 1. 鍥剧畝鍖栭樁娈?
        auto workGraph = graph;
        while (!workGraph.nodes().empty()) {
            bool simplified = false;
            
            // 灏濊瘯鎵惧埌鍙畝鍖栫殑鑺傜偣
            for (auto node : workGraph.nodes()) {
                if (workGraph.neighbors(node).size() < getAllPhysicalRegisters().size()) {
                    stack.push(node);
                    workGraph.removeNode(node);
                    simplified = true;
                    break;
                }
            }
            
            // 濡傛灉娌℃湁鍙畝鍖栫殑鑺傜偣锛岄€夋嫨婧㈠嚭
            if (!simplified) {
                auto node = workGraph.getMaxDegreeNode();
                stack.push(node);
                workGraph.removeNode(node);
            }
        }
        
        // 2. 鐫€鑹查樁娈?
        while (!stack.empty()) {
            auto node = stack.top();
            stack.pop();
            
            std::set<pRegister> usedColors;
            for (auto neighbor : graph.neighbors(node)) {
                if (coloring.count(neighbor)) {
                    usedColors.insert(coloring[neighbor]);
                }
            }
            
            // 鎵惧埌鍙敤鐨勯鑹?
            for (auto reg : getAllPhysicalRegisters()) {
                if (usedColors.count(reg) == 0) {
                    coloring[node] = reg;
                    break;
                }
            }
            
            // 濡傛灉娌℃湁鍙敤棰滆壊锛屾爣璁颁负婧㈠嚭
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
        // 1. 棣栧厛澶勭悊鎵€鏈夊凡鍒嗛厤鐨勮櫄鎷熷瘎瀛樺櫒
        for (auto& block : blocks) {
            for (auto& inst : block->getInstructions()) {
                inst->replaceVRegsWithPhysRegs(vregToPreg);
            }
        }
        
        // 2. 澶勭悊闇€瑕佹孩鍑虹殑瀵勫瓨鍣?
        for (const auto& spilledReg : spilledNodes) {
            size_t spillOffset = reserveSpillSlot();
            spillOffsets[spilledReg] = spillOffset;
            // 鍦ㄦ墍鏈変娇鐢ㄨ瀵勫瓨鍣ㄧ殑鍦版柟鎻掑叆鍔犺浇/瀛樺偍鎸囦护
            for (auto& block : blocks) {
                auto& instructions = block->getInstructions();
                
                std::vector<std::shared_ptr<Instruction>> instructionsToProcess;
                for (auto& inst : instructions) {
                    instructionsToProcess.push_back(inst);
                }
                
                for (auto& inst : instructionsToProcess) {
                    // 妫€鏌ユ寚浠ゆ槸鍚︿娇鐢ㄦ孩鍑虹殑瀵勫瓨鍣?
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
                        // 鍦ㄦ寚浠ゅ墠鎻掑叆鍔犺浇鎸囦护锛堝鏋滀娇鐢ㄨ瀵勫瓨鍣級
                        for (auto& reg : inst->getUseRegisters()) {
                            if (reg == spilledReg) {
                                auto tempReg = VirtualRegister::createTemporary(spilledReg->isFloatReg());
                                // 鏍规嵁瀵勫瓨鍣ㄧ被鍨嬮€夋嫨姝ｇ‘鐨勬寚浠ょ被鍨?
                                InstructionTy loadTy = spilledReg->isFloatReg() ? InstructionTy::FLW : InstructionTy::LW;
                                auto loadInst = std::make_shared<IInstruction>(
                                    loadTy,
                                    tempReg, 
                                    createStackPointer(), 
                                    std::make_shared<Immediate>(spillOffset)
                                );
                                block->insertBefore(inst, loadInst);
                                inst->replaceRegisterUse(spilledReg, tempReg);
                            }
                        }
                        
                        // 鍦ㄦ寚浠ゅ悗鎻掑叆瀛樺偍鎸囦护锛堝鏋滃畾涔夎瀵勫瓨鍣級
                        for (auto& reg : inst->getDefRegisters()) {
                            if (reg == spilledReg) {
                                auto tempReg = VirtualRegister::createTemporary(spilledReg->isFloatReg());
                                // 鏍规嵁瀵勫瓨鍣ㄧ被鍨嬮€夋嫨姝ｇ‘鐨勬寚浠ょ被鍨?
                                InstructionTy storeTy = spilledReg->isFloatReg() ? InstructionTy::FSW : InstructionTy::SW;
                                auto storeInst = std::make_shared<SInstruction>(
                                    storeTy,
                                    createStackPointer(),
                                    tempReg,
                                    std::make_shared<Immediate>(spillOffset)
                                );
                                block->insertAfter(inst, storeInst);
                                inst->replaceRegisterDef(spilledReg, tempReg);
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

