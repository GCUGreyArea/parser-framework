#ifndef TOO_MANY_CAPTURE_GROUPS_H
#define TOO_MANY_CAPTURE_GROUPS_H

#include <string>
#include "Generic.h"

namespace Exception {

class TooManyCaptureGroups : public Generic {
public:
    TooManyCaptureGroups(std::string rx)
        : Generic("Too many capture groups [" + rx + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//TOO_MANY_CAPTURE_GROUPS_H