#ifndef TOKEN_NOT_FOUND_H
#define TOKEN_NOT_FOUND_H

#include <string>
#include "Generic.h"

namespace Exception {

class TokenNotFound : public Generic {
public:
    TokenNotFound(std::string name)
        : Generic("Token not found [" + name + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//TOKEN_NOT_FOUND_H