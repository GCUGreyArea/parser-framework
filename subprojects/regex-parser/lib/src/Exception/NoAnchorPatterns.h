#ifndef NO_ANCHOR_PATTERNS_H
#define NO_ANCHOR_PATTERNS_H

#include <string>
#include "Generic.h"
namespace Exception {

class NoAnchorPatterns : public Generic {
public:
    NoAnchorPatterns(std::string id)
        : Generic("No anchor patterns for anchor [" + id + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//NO_ANCHOR_PATTERNS_H