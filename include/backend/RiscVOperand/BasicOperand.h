#include <string>

namespace backend{

    class Operand{
        public:
        std::string name;
        enum class OpType{
            Void = 0, 
            IntImm,   
            FloatImm, 
            IntReg, 
            FloatReg, 
            Memory,
            Label
        };
        private:
        OpType type;
        public:
        OpType getType(){
            return type;
        }
        std::string getTypeName(){
            switch (type)
            {
            case OpType::Void:
                return "void";
            case OpType::IntImm:
                return "int";
            case OpType::FloatImm:
                return "float";
            case OpType::IntReg:
                return "intreg";
            case OpType::FloatReg:
                return "floatreg";
            case OpType::Memory:
                return "memory";
            case OpType::Label:
                return "label";
            default:
                return "unknown";
            }
        }
        explicit Operand(OpType type) : type(type) {}
        ~Operand() = default;
        virtual std::string toString();
    };

}