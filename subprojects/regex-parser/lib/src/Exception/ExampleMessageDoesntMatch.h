#ifndef EXAMPLE_PATTERN_DOSNT_MATCH_H
#define EXAMPLE_PATTERN_DOSNT_MATCH_H

#include <string>
#include "Generic.h"

namespace Exception {

class ExampleMessageDoesntMatch : public Exception::Generic {
public:
    explicit ExampleMessageDoesntMatch(std::string message)
        : Generic(std::move(message)) {}
    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//EXAMPLE_PATTERN_DOSNT_MATCH_H
