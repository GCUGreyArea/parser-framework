#ifndef PATTERN_HAS_NO_TOKENS_H
#define PATTERN_HAS_NO_TOKENS_H

#include <string>
#include "Generic.h"

namespace Exception {

class PatternHasNoTokens : public Generic {
public:
    PatternHasNoTokens(std::string id)
        : Generic("Pattern has no tokens [" + id + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }

private:
    std::string mMsg;
};

};
#endif//PATTERN_HAS_NO_TOKENS_H