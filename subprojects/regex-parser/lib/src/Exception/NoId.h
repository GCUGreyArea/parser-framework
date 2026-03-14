#ifndef NO_ID_H
#define NO_ID_H

#include <string>
#include "Generic.h"

namespace Exception {

class NoId : public Generic {
public:
    NoId(std::string name)
        : Generic("No id [" + name + "]") {}

    const char * what() const throw () {
        return Generic::what();
    }

private:
    std::string mMsg;
};

};

#endif//NO_ID_H