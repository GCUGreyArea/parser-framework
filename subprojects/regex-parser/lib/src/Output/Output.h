#ifndef OUTPUT_DEF_H
#define OUTPUT_DEF_H

#include "Framework/Token.h"
#include "Framework/Rule.h"
#include "Framework/Pattern.h"
#include "Framework/Property.h"

class Output {
public:
    virtual void add_rule_value(Rule& t) = 0;
    virtual void add_rule_desc(Rule& t) = 0;
    virtual void add_pattern_value(Pattern& t) = 0;
    virtual void add_pattern_desc(Pattern& t) = 0;
    virtual void add_token_value(Token& t) = 0;
    virtual void add_token_desc(Token& t) = 0;
};

#endif//OUTPUT_DEF_H