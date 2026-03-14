#ifndef HTML_OUTPUT_HEADER
#define HTML_OUTPUT_HEADER

#include "Output.h"
#include "ctml.hpp"

class HTMLOut : public Output {
public:
    virtual void add_rule_value(Rule& r) {
        std::cout << "NOT IMPLIMENTED YET!" << std::endl;
    }

    virtual void add_rule_desc(Rule& t) {};
    virtual void add_pattern_value(Pattern& t) {};
    virtual void add_pattern_desc(Pattern& t) {};
    virtual void add_token_value(Token& t) {};
    virtual void add_token_desc(Token& t) {};

    friend std::ostream& operator<<(std::ostream& os, const HTMLOut& obj) {
        os << "NOT IMPLIMETED YET!" << std::endl;
        return os;
    }

private:
    CTML::Document mDoc;
};

#endif//HTML_OUTPUT_HEADER