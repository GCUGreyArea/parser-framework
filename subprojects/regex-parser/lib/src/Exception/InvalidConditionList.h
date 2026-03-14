#ifndef INVALID_CONDITION_LIST_H
#define INVALID_CONDITION_LIST_H


#include <string>
#include "Generic.h"

namespace Exception {

class InvalidConditionList : public Generic {
public:
    InvalidConditionList()
        : Generic("Condition list on lhs cannot be empty") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

}

#endif//INVALID_CONDITION_LIST_H