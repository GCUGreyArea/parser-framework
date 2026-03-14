#ifndef INVALID_CAPTURE_GROUPS_H
#define INVALID_CAPTURE_GROUPS_H

#include <string>
#include "Generic.h"

namespace Exception {

class InvalidCaptureGroups : public Generic {
public:
    InvalidCaptureGroups(std::string rx)
        : Generic("Not a valid capture group [" + rx + "]") {}
    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//INVALID_CAPTURE_GROUPS_H