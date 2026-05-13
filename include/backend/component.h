#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Value.h"
#include "RiscVOperand/OpRegister.h"
#include "Instructions.h"
#include "instruction.h"
#include "basicblock.h"
#include "module.h"
#include "type.h"
#include "globalvalue.h"
#include "AsmConstant.h"

namespace backend {

inline std::string sanitizeSymbolName(const std::string& rawName) {
    if (!rawName.empty() && rawName.front() == '@') {
        return rawName.substr(1);
    }
    return rawName;
}

using rRegister = std::shared_ptr<VirtualRegister>;
using pRegister = std::shared_ptr<PhysicalRegister>;

// 干涉图类型定�?
class InterferenceGraph {
public:
    // 邻接表：每个虚拟寄存�?-> 与之冲突的虚拟寄存器集合
    std::unordered_map<rRegister, std::unordered_set<rRegister>> adjList;

    void addEdge(const rRegister& a, const rRegister& b) {
        if (a == b) return;
        adjList[a].insert(b);
        adjList[b].insert(a);
    }

    const std::unordered_set<rRegister>& neighbors(const rRegister& reg) const {
        static std::unordered_set<rRegister> empty;
        auto it = adjList.find(reg);
        return it == adjList.end() ? empty : it->second;
    }

    std::vector<rRegister> nodes() const {
        std::vector<rRegister> result;
        for (const auto& [reg, _] : adjList) result.push_back(reg);
        return result;
    }
    
    // 移除节点
    void removeNode(const rRegister& reg) {
        // 先移除该节点所有邻接关�?
        for (auto& neighbor : adjList[reg]) {
            adjList[neighbor].erase(reg);
        }
        // 再移除节点本�?
        adjList.erase(reg);
    }
    
    // 获取节点的度�?
    size_t degree(const rRegister& reg) const {
        auto it = adjList.find(reg);
        return it == adjList.end() ? 0 : it->second.size();
    }
    
    // 检查两个节点是否相�?
    bool isAdjacent(const rRegister& a, const rRegister& b) const {
        auto it = adjList.find(a);
        return it != adjList.end() && it->second.count(b) > 0;
    }
    
    // 获取图中度数最小的节点
    rRegister getMinDegreeNode() const {
        if (adjList.empty()) return nullptr;
        
        auto minIt = adjList.begin();
        for (auto it = adjList.begin(); it != adjList.end(); ++it) {
            if (it->second.size() < minIt->second.size()) {
                minIt = it;
            }
        }
        return minIt->first;
    }
    
    // 获取图中度数最大的节点
    rRegister getMaxDegreeNode() const {
        if (adjList.empty()) return nullptr;
        
        auto maxIt = adjList.begin();
        for (auto it = adjList.begin(); it != adjList.end(); ++it) {
            if (it->second.size() > maxIt->second.size()) {
                maxIt = it;
            }
        }
        return maxIt->first;
    }
    
    // 检查图是否为空
    bool empty() const {
        return adjList.empty();
    }
    
    // 获取图的大小(节点�?
    size_t size() const {
        return adjList.size();
    }
};

class RegAlloca;
class AsmFunction;
class AsmModule;

class AsmGlobalVariable {
public:
    std::string name;
    int size;
    bool isConst;
    bool isInitialized;
    int elementNum;
    std::vector<GlobalConstant> constDataList;
    int align;
    bool isSmallSection;

    AsmGlobalVariable(IR::GlobalVariable* globalVar)
        : name(sanitizeSymbolName(globalVar->getIRName())),
          isConst(globalVar->isPointingConst()),
          isInitialized(globalVar->isInitialized()) {
        
        if (!globalVar->getType()->isPointerTy()) {
            throw std::runtime_error("Global variable must be pointer type");
        }
        
        auto baseType = globalVar->getType()->getPointerBase();
        elementNum = 1;

        if (globalVar->isInitialized()) {
            auto init = globalVar->getOperand(0);
            if (init->isConstantInt32()) {
                size = 4;
                align = 2;
                isSmallSection = true;
                constDataList.emplace_back(
                    static_cast<IR::ConstantInt32*>(init)->getValue());
            } else if (init->isConstantFloat()) {
                size = 4;
                align = 2;
                isSmallSection = true;
                constDataList.emplace_back(
                    static_cast<IR::ConstantFloat*>(init)->getValue());
            } else if (init->isConstantArray()) {
                auto array = static_cast<IR::ConstantArray*>(init);
                elementNum = array->getType()->getArraySize();
                size = array->getType()->size();
                align = 3;
                isSmallSection = false;
                initializeArrayData(array);
            }
        } else {
            if (baseType->isInt32Ty() || baseType->isFloatTy()) {
                size = 4;
                align = 2;
                isSmallSection = true;
                constDataList.push_back(GlobalConstant::createZeroFill(4));
            } else if (baseType->isArrayTy()) {
                elementNum = static_cast<const IR::ArrayType*>(baseType)->getArraySize();
                size = baseType->size();
                align = 3;
                isSmallSection = false;
                constDataList.push_back(GlobalConstant::createZeroFill(size));
            }
        }
    }

    std::string output() const {
        std::string s;
        s += "\t.globl " + name + "\n";
        
        if (isConst) {
            s += isSmallSection ? "\t.section .srodata,\"a\"" : "\t.section .rodata";
        } else if (isInitialized) {
            s += isSmallSection ? "\t.section .data,\"aw\"" : "\t.section .data";
        } else {
            s += "\t.section .bss";
        }
        
        s += "\n\t.align " + std::to_string(1 << align) + "\n";
        s += name + ":\n";
        
        for (auto& data : constDataList) {
            s += data.output();
        }
        
        return s;
    }

private:
    void initializeArrayData(IR::ConstantArray* array) {
        auto arrayType = array->getType();
        auto baseType = arrayType->getArrayBase();
        int elementSize = baseType->size() / arrayType->getArraySize();

        int lastInitIndex = -1;
        for (auto& [index, constant] : array->elements) {
            if ((int)index != lastInitIndex + 1) {
                constDataList.push_back(GlobalConstant::createZeroFill(
                    (index - lastInitIndex - 1) * elementSize));
            }

            if (constant->isConstantInt32()) {
                constDataList.emplace_back(
                    static_cast<IR::ConstantInt32*>(constant)->getValue());
            } else if (constant->isConstantFloat()) {
                constDataList.emplace_back(
                    static_cast<IR::ConstantFloat*>(constant)->getValue());
            } else if (constant->isConstantArray()) {
                initializeArrayData(static_cast<IR::ConstantArray*>(constant));
            }

            lastInitIndex = index;
        }

        if (lastInitIndex + 1 < arrayType->getArraySize()) {
            constDataList.push_back(GlobalConstant::createZeroFill(
                (arrayType->getArraySize() - lastInitIndex - 1) * elementSize));
        }
    }
};

class AsmBasicBlock {
public:
    AsmBasicBlock(AsmFunction* func, IR::BasicBlock* block)
        : parentFunction(func),
          irBlock(block),
          label(std::make_unique<Label>(block->getIRName())) {}
    
    void addInstruction(std::shared_ptr<Instruction> inst) {
        instructions.push_back(std::move(inst));
    }
    AsmFunction* getParentFunction() const { return parentFunction; }
    std::vector<AsmBasicBlock*> getSuccessors();
    std::shared_ptr<Instruction> convertInstruction(IR::Instruction* inst);
    std::shared_ptr<Instruction> convertBinaryInstruction(IR::BinaryInstruction* inst);
    std::shared_ptr<Instruction> convertUnaryInstruction(IR::UnaryInstruction* inst);
    std::shared_ptr<Instruction> convertLoadInstruction(IR::LoadInstruction* inst);
    std::shared_ptr<Instruction> convertStoreInstruction(IR::StoreInstruction* inst);
    std::shared_ptr<Instruction> convertBranchInstruction(IR::BranchInstruction* inst);
    std::shared_ptr<Instruction> convertReturnInstruction(IR::ReturnInstruction* inst);
    std::shared_ptr<Instruction> convertCastInstruction(IR::CastInstruction* inst);
    std::shared_ptr<Instruction> convertCmpInstruction(IR::CmpInstruction* inst);
    std::shared_ptr<Instruction> convertCallInstruction(IR::CallInstruction* inst);
    std::shared_ptr<Instruction> convertGetElementPtrInstruction(IR::GetElementPtrInstruction* inst);
    std::shared_ptr<Instruction> convertGEPInstruction(IR::GetElementPtrInstruction* inst);
    std::shared_ptr<Instruction> convertAllocaInstruction(IR::AllocaInstruction* inst);
    // void processPhiNodes() {
    // for (auto& inst : instructions) {
    //     if (auto phi = dynamic_cast<IR::PhiInstruction*>(inst.get())) {
    //         // 在基本块开头为phi节点生成move指令
    //         auto dst = VirtualRegister::create(phi);
    //         for (auto& [val, bb] : phi->getIncomingValue()) {
    //             if (bb == this->irBlock) {
    //                 auto src = VirtualRegister::create(val);
    //                 addInstruction(std::make_shared<RInstruction>(
    //                     dst->isFloatReg() ? InstructionTy::FSGNJ_S : InstructionTy::ADD,
    //                     dst, src, src));
    //                 break;
    //                 }
    //             }
    //         }
    //     }
    // }
    const std::vector<std::shared_ptr<Instruction>>& getInstructions() const { 
        return instructions; 
    }

    void initializeInstructions(){
        // std::cout<< "Initializing instructions, count: " << irBlock->getVectorInstructions().size() << std::endl;
        for (auto& inst : irBlock->getVectorInstructions()) {
            auto asmInst = convertInstruction(inst);
            if (asmInst) {
                addInstruction(asmInst);
            }
        }
    }
    void insertBefore(std::shared_ptr<Instruction> target, std::shared_ptr<Instruction> inst) {
        for (auto it = instructions.begin(); it != instructions.end(); ++it) {
            if (it->get() == target.get()) {
                instructions.insert(it, inst);
                break;
            }
        }
    }

    void insertAfter(std::shared_ptr<Instruction> target, std::shared_ptr<Instruction> inst) {
        for (auto it = instructions.begin(); it != instructions.end(); ++it) {
            if (it->get() == target.get()) {
                instructions.insert(it + 1, inst);
                break;
            }
        }
    }
    
    std::string output() const {
        std::string s;
        s += label->toString() + ":\n";
        for (const auto& inst : instructions) {
            s += "\t" + inst->output();
        }
        return s;
    }

    std::set<rRegister> liveIn;
    std::set<rRegister> liveOut;
private:
    IR::BasicBlock* irBlock;
    AsmFunction* parentFunction;
    std::unique_ptr<Label> label;
    std::vector<std::shared_ptr<Instruction>> instructions;
    
};

class AsmFunction {
public:
    AsmFunction(AsmModule* module, IR::Function* func)
        : irFunction(func),
          label(new Label(sanitizeSymbolName(func->getIRName()), true)),
          regAlloca(nullptr),
          needsReturnAddressSave(functionHasCallInstruction()) {
        // 初始化所有AsmBasicBlock并建立映�?
        for (IR::ListNode *i = irFunction->blocks().begin(); i != irFunction->blocks().end(); i = i->nextNode()) {
            auto block = static_cast<IR::BasicBlock*>(i);
            auto asmBlock = new AsmBasicBlock(this, block);
            ir2asmBlock[block] = asmBlock;
            blocks.push_back(std::unique_ptr<AsmBasicBlock>(asmBlock));
        }
        for (auto& block : blocks) {
            block->initializeInstructions();
        }
        initReturnRegister();
        initArguments();
    }

    void allocateRegisters(){
        livenessAnalysis(); // 步骤1
        auto graph = buildInterferenceGraph(); // 步骤2
        auto vregToPreg = colorGraph(graph);   // 步骤3
        rewriteInstructions(vregToPreg);  
    }
    void addBlock(AsmBasicBlock* block) {
        blocks.push_back(std::unique_ptr<AsmBasicBlock>(block));
    }

    void addInstruction(std::shared_ptr<Instruction> inst) {
        instructions.addInstruction(inst);
    }

    bool isLibFunc() const {
        return irFunction->isBuiltinFunction();
    }

     AsmBasicBlock* getAsmBasicBlock(IR::BasicBlock* irBlock) {
        auto it = ir2asmBlock.find(irBlock);
        if (it != ir2asmBlock.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::string output() {
        if (isLibFunc()) {
            return "";
        }
        allocateRegisters();
        std::string s;
        s += "\t.globl " + label->getLabelName() + "\n";
        s += "\t.type " + label->getLabelName() + ", @function\n";
        s += label->getLabelName() + ":\n";
        if (getStackFrameSize() > 0) {
            s += "\taddi sp, sp, -" + std::to_string(getStackFrameSize()) + "\n";
            if (shouldSaveReturnAddress()) {
                s += "\tsw ra, " + std::to_string(getReturnAddressOffsetFromFixedFrameBase()) + "(sp)\n";
            }
        }

        // 输出函数�?
        // std::cout<<"block count: " << blocks.size() << std::endl;
        for (auto& block : blocks) {
            s += block->output();
        }
        
        s += "\t.size " + label->getLabelName() + ", .-" + label->getLabelName() + "\n";
        return s;
    }

    void recordStackAllocation(IR::AllocaInstruction* inst, size_t size, size_t alignment) {
        size_t alignedOffset = alignTo(nextStackOffset, alignment);
        stackSlotOffsets[inst] = alignedOffset;
        nextStackOffset = alignedOffset + size;
        stackAllocations[inst] = {size, alignment};
    }
    
    // 获取栈帧总大�?
    size_t getStackFrameSize() const {
        size_t total = getFixedFrameBytes();
        return (total + 15) / 16 * 16;  // 16字节对齐
    }
    size_t getAllocatedStackBytes() const {
        return nextStackOffset;
    }
    size_t reserveSpillSlot() {
        return getAllocatedStackBytes() + (spillSlotCount++ * 4);
    }
    size_t getFixedFrameBytes() const {
        return getAllocatedStackBytes() + spillSlotCount * 4 + (needsReturnAddressSave ? 4 : 0);
    }
    size_t getReturnAddressOffsetFromFixedFrameBase() const {
        return getAllocatedStackBytes() + spillSlotCount * 4;
    }
    bool shouldSaveReturnAddress() const {
        return needsReturnAddressSave;
    }
    size_t getAllocaOffset(IR::AllocaInstruction* inst) const {
        auto it = stackSlotOffsets.find(inst);
        if (it == stackSlotOffsets.end()) {
            throw std::runtime_error("Unknown stack slot for alloca");
        }
        return it->second;
    }
    bool hasAllocaOffset(IR::AllocaInstruction* inst) const {
        return stackSlotOffsets.find(inst) != stackSlotOffsets.end();
    }
    static rRegister createStackPointer() {
        return VirtualRegister::createStackPointer();
    }

    void markForSpilling(const rRegister& reg) {
        spilledNodes.insert(reg);
    }

    std::set<rRegister> getSpilledNodes() const {
        return spilledNodes;
    }
private:
    IR::Function* irFunction;
    std::unique_ptr<Label> label;
    std::vector<std::unique_ptr<AsmBasicBlock>> blocks;
    InstructionList instructions;
    RegAlloca* regAlloca;
    rRegister returnReg;
    std::vector<rRegister> args; 
    std::map<IR::Argument*, rRegister> asmArgMap; 
    std::unordered_map<IR::AllocaInstruction*, std::pair<size_t, size_t>> stackAllocations;
    std::unordered_map<IR::AllocaInstruction*, size_t> stackSlotOffsets;
    std::set<rRegister> spilledNodes;
    std::unordered_map<IR::BasicBlock*, AsmBasicBlock*> ir2asmBlock;
    size_t spillSlotCount = 0;
    size_t nextStackOffset = 0;
    bool needsReturnAddressSave = false;


    void initReturnRegister() {
        if (!irFunction->getReturnType()->isVoidTy()) {
            returnReg = VirtualRegister::createReturnValue(
                irFunction->getReturnType()->isFloatTy());
        }
    }

    void livenessAnalysis();
    InterferenceGraph buildInterferenceGraph();
    std::unordered_map<rRegister, pRegister> colorGraph(const InterferenceGraph& graph);
    void rewriteInstructions(std::unordered_map<rRegister, pRegister>& vregToPreg);
    
    void initArguments() {
        for (auto& arg : irFunction->args()) {
            auto reg = VirtualRegister::create(arg, arg->getType()->isFloatTy());
            args.push_back(reg);
            asmArgMap[arg] = reg;
        }
    }
    bool functionHasCallInstruction() const {
        for (IR::ListNode *i = irFunction->blocks().begin(); i != irFunction->blocks().end(); i = i->nextNode()) {
            auto *block = static_cast<IR::BasicBlock*>(i);
            for (auto *inst : block->getVectorInstructions()) {
                if (inst->getOpcode() == IR::Instruction::Call) {
                    return true;
                }
            }
        }
        return false;
    }
    static size_t alignTo(size_t value, size_t alignment) {
        if (alignment == 0) {
            return value;
        }
        return (value + alignment - 1) / alignment * alignment;
    }

    std::tuple<std::set<rRegister>, std::set<rRegister>> computeUseDef(const std::unique_ptr<AsmBasicBlock>& block);
    std::tuple<std::set<rRegister>, std::set<rRegister>> computeUseDefForInstruction(const std::shared_ptr<Instruction>& inst);
};

class AsmModule {
public:
    AsmModule(IR::Module* module) : irModule(module) {
        for (auto& global : module->getGlobalVariableList()) {
            globals.push_back(std::move(std::make_unique<AsmGlobalVariable>(global)));
        }
        
        for (auto& func : module->getFunctionList()) {
            functions.push_back(std::move(std::make_unique<AsmFunction>(this, func)));
        }
    }

    void addFunction(std::unique_ptr<AsmFunction> func) {
        functions.push_back(std::move(func));
    }

    void addGlobal(std::unique_ptr<AsmGlobalVariable> global) {
        globals.push_back(std::move(global));
    }

    const std::vector<std::unique_ptr<AsmFunction>>& getFunctions() const {
        return functions;
    }

    std::string output() {
        std::string s;
        s += "\t.text\n";
        
        // 输出全局变量
        for (auto& global : globals) {
            s += global->output();
        }
        
        // 输出函数
        for (auto& func : functions) {
            if (!func->isLibFunc()) {
                s += func->output();
            }
        }
        
        return s;
    }

    void gen() {
        for (auto& global : globals) {
            global->output();
        }
        
        for (auto& func : functions) {
            func->output();
        }
    }
private:
    IR::Module* irModule;
    std::vector<std::unique_ptr<AsmFunction>> functions;
    std::vector<std::unique_ptr<AsmGlobalVariable>> globals;
};

} // namespace backend

