#ifndef INVLID_ID_H
#define INVLID_ID_H

#include <string>
#include "Generic.h"

namespace Exception {

class InvalidId : public Generic {
public:
    InvalidId(std::string id)
        : Generic("Not a valid id [" + id + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

}
#endif//INVLID_ID_H