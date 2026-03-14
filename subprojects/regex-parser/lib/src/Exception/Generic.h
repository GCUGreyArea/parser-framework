#ifndef GENERIC_H
#define GENERIC_H

#include <exception>
#include <string>

namespace Exception {

class Generic : public std::exception {
public:
    Generic(std::string msg)
        : mMsg(msg) {}
    const char * what() const throw () {
        return mMsg.c_str();
    }

private:
    std::string mMsg;
};

};

#endif//GENERIC_H