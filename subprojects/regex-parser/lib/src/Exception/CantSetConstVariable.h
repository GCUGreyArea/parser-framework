#ifndef CANT_SET_CONST_EXP_EXCEPTION_H
#define CANT_SET_CONST_EXP_EXCEPTION_H

#include <string>
#include "Generic.h"

namespace Exception {

class CantSetConstVariable : public Exception::Generic {
public:
    CantSetConstVariable(std::string id)
        : Generic("Attempt to set const variable [" + id + "]") {}
    const char * what() const throw () {
        return Generic::what();
    }

private:
    std::string mMsg;
};

};

#endif//CANT_SET_CONST_EXP_EXCEPTION_H