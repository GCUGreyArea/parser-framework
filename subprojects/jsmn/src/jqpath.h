#ifndef JQ_DEFS
#define JQ_DEFS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>

#include "hash.h"

enum jq_operator {
    JQ_NOT_SET = 0,        // No value has been set yet
    JQ_EQUALS,             // Equality operator represented by = 
    JQ_NOT_EQUALS,         // Non equality operator != 
    JQ_GREATER_THAN,       // Less than operator <
    JQ_LESS_THAN,          // Greater than operator >
    JQ_MATCH,              // Rrepresents a regex that must be defined as R"regex text " 
    JQ_CLOSE_TO            // Use Levenshtein distance
};

union jqvalue_u {
    bool bool_val;
    int int_val;
    float float_val;
    char * string_val;
};

enum jqvaltype_e {
    JQ_NO_VAL,
    JQ_INT_VAL,
    JQ_FLOAT_VAL,
    JQ_STRING_VAL,
    JQ_BOOL_VAL
};

struct jqvalue {
    enum jqvaltype_e type;
    union jqvalue_u value;
};

struct span {
    int start;
    int length;
};

struct jqpath {
    unsigned int hash;      //! The hash of the path
    unsigned int depth;     //! The depth of the path
    bool rendered;          //! Has the vakue been rendered from the string
    struct span sp;         //! Place in the string for the value
    enum jq_operator op;    //! The operator to use if there is an eequality argument
    struct jqvalue value;   //! The string representing the equality argument
};

struct jqpath* jqpath_parse_string(const char * str);
struct jqpath* jsonpath_parse_string(const char * str);
struct jqpath* path_parse_string(const char * str);
struct jqpath* jqpath_get_path();
void jqpath_close_path(struct jqpath* path);

bool jqpath_value_equals_int(struct jqpath * path,int val);
bool jqpath_value_equals_float(struct jqpath * path,float val);
bool jqpath_value_equals_string(struct jqpath * path,char * val);


#ifdef __cplusplus
}
#endif

#endif//JQ_DEFS
