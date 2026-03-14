#ifndef NO_NAME_H
#define NO_NAME_H

#include <string>
#include "Generic.h"

namespace Exception {

class NoName : public Generic {
public:
    NoName(std::string id)
        : Generic("Notname suplied [" + id + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }

private:
    std::string mMsg;
};
};

#endif//NO_NAME_H