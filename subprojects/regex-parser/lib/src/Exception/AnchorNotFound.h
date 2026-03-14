#ifndef ANCHOR_NOT_FOUND_H
#define ANCHOR_NOT_FOUND_H

#include <string>
#include "Generic.h"

namespace Exception {

class AnchorNotFound : public Exception::Generic {
public:
    AnchorNotFound(std::string id)
        : Generic("No anchor patterns for anchor [" + id + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//ANCHOR_NOT_FOUND_H