#ifndef INVALID_OPERATOR_EXCEPTION_H
#define INVALID_OPERATOR_EXCEPTION_H

#include <string>
#include "Generic.h"

namespace Exception {

class InvalidOperator : public Generic {
public:
    InvalidOperator(std::string op)
        : Generic("Not a valid operator [" + op + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }

private:
    std::string mMsg;
};

}

#endif//INVALID_OPERATOR_EXCEPTION_H