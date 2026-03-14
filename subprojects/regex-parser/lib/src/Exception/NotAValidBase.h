#ifndef NOT_A_VALID_NUMBER_STRING_H
#define NOT_A_VALID_NUMBER_STRING_H

#include <string>
#include "Generic.h"

namespace Exception {

class NotAValidBase : public Generic {
public:
    NotAValidBase(std::string str)
        : Generic("Not a valid number base, did you call is_number first? [base " + str + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

};

#endif//NOT_A_VALID_NUMBER_STRING_H