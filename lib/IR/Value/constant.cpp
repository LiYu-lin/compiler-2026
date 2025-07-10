#include "ir/Value/constant.h"

namespace IR
{
    Constant *Constant::getZeroValueForType(pType type)
    {
        if (type->isInt32Ty())
            return new ConstantInt32(0);
        else if (type->isFloatTy())
            return new ConstantFloat(0.0);
        else if (type->isArrayTy())
            return new ConstantArray(type);
        else
        {
            return nullptr;
        }
    }
}