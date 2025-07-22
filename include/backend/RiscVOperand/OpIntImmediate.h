#include "Basicoperand.h"
#include <memory>
#include <vector>
#include <utility>

namespace backend{

    class Immediate : public Operand {
    public:
        enum class ImmType {
            Int,
            Float,
            Join,
            Split
        };
        //high-20，low-12
        enum class SplitPartition { HI, LO };

        Immediate(int value) 
            : Operand(OpType::IntImm), value(value), immType(ImmType::Int) {}
            
        Immediate(float value) 
            : Operand(OpType::FloatImm), floatValue(value), immType(ImmType::Float) {}
            
        Immediate(bool isJoin, std::shared_ptr<Immediate> origin = nullptr, 
                SplitPartition part = SplitPartition::LO)
            : Operand(isJoin ? OpType::Label : OpType::Memory),
            immType(isJoin ? ImmType::Join : ImmType::Split),
            origin(std::move(origin)), part(part) {}

        ImmType getImmType() const { return immType; }
        
        int getIntValue() const { return value; }
        float getFloatValue() const { return floatValue; }
        

        SplitPartition getSplitPart() const { return part; }
        std::shared_ptr<Immediate> getOrigin() const { return origin; }

        void addJoinedValue(std::shared_ptr<Immediate> imm) {
            if (immType == ImmType::Join) {
                joinedValues.push_back(std::move(imm));
            }
        }
        const std::vector<std::shared_ptr<Immediate>>& getJoinedValues() const {
            return joinedValues;
        }

    private:
        union {
            int value;
            float floatValue;
        };
        ImmType immType;
        
        std::shared_ptr<Immediate> origin;
        SplitPartition part;
        std::vector<std::shared_ptr<Immediate>> joinedValues;
    };

    inline std::shared_ptr<Immediate> createIntImmediate(int value) {
        return std::make_shared<Immediate>(value);
    }
    inline std::shared_ptr<Immediate> createFloatImmediate(float value) {
        return std::make_shared<Immediate>(value);
    }
    inline std::shared_ptr<Immediate> createSplitImmediate(
        std::shared_ptr<Immediate> origin, Immediate::SplitPartition part) {
        return std::make_shared<Immediate>(false, std::move(origin), part);
    }

    inline std::shared_ptr<Immediate> createJoinImmediate() {
        return std::make_shared<Immediate>(true);
    }

}