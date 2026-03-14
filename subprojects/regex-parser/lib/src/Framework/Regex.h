#ifndef REGEX_H
#define REGEX_H

#include <re2/re2.h>
#include <cstddef>
#include <string>

#include "Utils/Utils.h"
#include "Exception/BadRegex.h"
#include "Exception/TooManyCaptureGroups.h"
#include "Match.h"

class Regex {
public:
    Regex(std::string rx);
    Regex(const Regex& r);

    bool match(Match& m);
    bool match(Match& m, std::string assert);
    void validate();
    std::string as_string() const {return {mResult.begin(),mResult.end()};}

    /**
     * @brief Convert the result of a regex to a valid number based on parsed format
     *
     * @return int
     */
    int as_int() const {
        std::string str = {mResult.begin(),mResult.end()};
        unsigned int base = Numbers::is_number(str);
        if(base > 0)
            return Numbers::to_int(str,base);

        throw Exception::NotAValidBase(str);
    }
    size_t current_offset(Match& m) {return mResult.end() - m.data();}

    std::string get_regex_string() {return mRxStr;}

protected:
    std::string mRxStr;
    re2::StringPiece mResult;
};

#endif//REGEX_H