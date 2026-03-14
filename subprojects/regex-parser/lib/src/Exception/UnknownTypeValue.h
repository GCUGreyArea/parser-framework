#ifndef UNKNOWN_TYPE_VALUE_H
#define UNKNOWN_TYPE_VALUE_H

#include <string>
#include "Generic.h"

namespace Exception {

class UnknownTypeValue : public Generic {
public:
    UnknownTypeValue(std::string type)
        : Generic("Too many capture groups [" + type + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//UNKNOWN_TYPE_VALUE_H