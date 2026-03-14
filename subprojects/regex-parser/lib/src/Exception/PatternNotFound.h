#ifndef PATTERN_NOT_FOUND_H
#define PATTERN_NOT_FOUND_H

#include <string>
#include "Generic.h"

namespace Exception {

class PatternNotFound : public Generic {
public:
    PatternNotFound(std::string id)
        : Generic("Pattern not found [" + id + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//PATTERN_NOT_FOUND_H