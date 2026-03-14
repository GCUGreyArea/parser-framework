#ifndef UNKNOWN_MATCH_TYPE_H
#define UNKNOWN_MATCH_TYPE_H

#include <exception>
#include <string>

namespace Exception {

class UnknownMatchType : public Generic {
public:
    UnknownMatchType(std::string type)
        : Generic("Too many capture groups [" + type + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }
};

};
#endif//UNKNOWN_MATCH_TYPE_HH