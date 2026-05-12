#include "use.h"
#include "Value.h"
#include "user.h"
#include <algorithm>
namespace IR
{
    Use *Use::create(Value *val, User *user)
    {
        Use *use = new Use(val, user);
        val->addUsage(use);
        user->uses.push_back(use);
        return use;
    }

    void Use::setValue(Value *value)
    {
        if (val != nullptr) {
            val->useList.remove(this);
            val = value;
            value->addUsage(this);
            int idx = std::find(user->uses.begin(), user->uses.end(), this) - user->uses.begin();
            user->operands[idx] = value;
        } else {
            //Error::Error(__PRETTY_FUNCTION__, "val is nullptr");
        }
    }
}