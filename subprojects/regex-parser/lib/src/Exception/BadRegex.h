#ifndef BAD_REGEX_H
#define BAD_REGEX_H

#include <string>
#include "Generic.h"

namespace Exception {

class BadRegex : public Exception::Generic {
public:
    BadRegex(std::string rx)
        : Generic("Bad regular expression [" + rx + "]") {}

    virtual const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//BAD_REGEX_H