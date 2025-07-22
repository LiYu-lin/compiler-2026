#include "BasicOperand.h"

namespace backend{

    class Register{
        public:
            Register(int id) : id(id) {}
            int getId() const { return id; }
        private:
            int id;
    };

}