#ifndef GENERAL_EXCEPTION_H
#define GENERAL_EXCEPTION_H

#include <string>
#include "Generic.h"

namespace Exception {

class General : public Generic {
public:
    General(std::string msg)
        : Generic(msg) {}
    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//GENERAL_EXCEPTION_H