#include "module.h"
#include "type.h"
#include "basicblock.h"
#include "instruction.h"
#include <iostream>
namespace IR
{
    Module::Module(std::string name) : name(name)
    {
        addBuiltinFunction("memset",
                           ExternalFunction::create(Type::getVoidType(), {PointerType::getPointerType(Type::getI32Type()), Type::getI32Type(), Type::getI32Type()}, "memset"));
        addBuiltinFunction("getint",
                           ExternalFunction::create(Type::getI32Type(), {}, "getint"));
        addBuiltinFunction("getch",
                           ExternalFunction::create(Type::getI32Type(), {}, "getch"));
        addBuiltinFunction("getarray",
                           ExternalFunction::create(Type::getI32Type(), {PointerType::getPointerType(Type::getI32Type())}, "getarray"));
        addBuiltinFunction("getfloat",
                           ExternalFunction::create(Type::getFloatType(), {}, "getfloat"));
        addBuiltinFunction("getfarray",
                           ExternalFunction::create(Type::getI32Type(), {PointerType::getPointerType(Type::getFloatType())}, "getfarray"));
        addBuiltinFunction("putint",
                           ExternalFunction::create(Type::getVoidType(), {Type::getI32Type()}, "putint"));
        addBuiltinFunction("putch",
                           ExternalFunction::create(Type::getVoidType(), {Type::getI32Type()}, "putch"));
        addBuiltinFunction("putarray",
                           ExternalFunction::create(Type::getVoidType(), {Type::getI32Type(), PointerType::getPointerType(Type::getI32Type())}, "putarray"));
        addBuiltinFunction("putfloat",
                           ExternalFunction::create(Type::getVoidType(), {Type::getFloatType()}, "putfloat"));
        addBuiltinFunction("putfarray",
                           ExternalFunction::create(Type::getVoidType(), {Type::getI32Type(), PointerType::getPointerType(Type::getFloatType())}, "putfarray"));
        addBuiltinFunction("starttime",
                           ExternalFunction::create(Type::getVoidType(), {}, "starttime"));
        addBuiltinFunction("stoptime",
                           ExternalFunction::create(Type::getVoidType(), {}, "stoptime"));
        addBuiltinFunction("_sysy_starttime",
                           ExternalFunction::create(Type::getVoidType(), {Type::getI32Type()}, "_sysy_starttime"));
        addBuiltinFunction("_sysy_stoptime",
                           ExternalFunction::create(Type::getVoidType(), {Type::getI32Type()}, "_sysy_stoptime"));
    }

    void Module::addBuiltinFunction(std::string name, ExternalFunction *func)
    {
        builtinFunctions[name] = func;
        functionList.push_back(func);
    }

    Function *Module::getBuiltinFunction(std::string name)
    {
        auto idx = builtinFunctions.find(name);
        // if (idx == builtinFunctions.end())
        //     Error::Error(__PRETTY_FUNCTION__, "Builtin function not found");
        return (*idx).second;
    }

    void Module::gen(std::ostream &os)
    {
        os << "module " << name << std::endl;

        for (auto &global : globalVariableList)
        {
            global->emitIR(os);
        }
        if (hasGlobalInit()) {
        os << "global.init:" << std::endl;
        auto instructions = globalInitBlock->getVectorInstructions();
        for (auto &inst : instructions) {
            inst->emitIR(os);
            }
        }
        for (auto &function : functionList)
        {
            function->emitIR(os);
        }
    }

    void Module::emitUse(std::ostream &os)
    {
        // for (auto &builtinFunctions : builtinFunctions)
        // {
        //     builtinFunctions.second->emitUse(os);
        // }
        for (auto &global : globalVariableList)
        {
            global->emitUse(os);
        }
        for (auto &function : functionList)
        {
            function->emitUse(os);
        }
    }
}
