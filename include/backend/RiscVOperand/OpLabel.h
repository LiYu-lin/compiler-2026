#pragma once
#include "Basicoperand.h"

namespace backend {
    class Label : public Operand {
    private:
        std::string labelname;
        bool isGlobal = false;
    public:
        Label(const std::string &label) : Operand(OpType::Label), labelname(label) {}
        Label(const std::string &label, bool global) 
            : Operand(OpType::Label), labelname(label), isGlobal(global) {}
        std::string getLabelName() const { return labelname; }
        void setLabelName(const std::string &name) { labelname = name; }
        bool isGlobalLabel() const { return isGlobal; }
        void setGlobalLabel(bool global) { isGlobal = global; }
        std::string toString() const override {
        return labelname;
    }
    };

}