#ifndef NO_REGEX_PATTERN_H
#define NO_REGEX_PATTERN_H

#include <exception>
#include <string>

namespace Exception {

class NoRegexPattern : public Generic {
public:
    NoRegexPattern()
        : Generic("No regex pattern found") {}

    const char * what() const throw () {
        return Generic::what();
    }

};
};

#endif//NO_REGEX_PATTERN_H