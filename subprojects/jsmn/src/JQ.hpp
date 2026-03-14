#ifndef JQ_PATH
#define JQ_PATH

#include <string>

#include "jqpath.h"
#include "string_funcs.h"


struct jsmntok;
typedef struct jsmntok jsmntok_t; 

class JQ {
public:
    JQ();
    JQ(struct jqpath& val);
    JQ(int depth, unsigned int hash, span sp);
    JQ(int depth, unsigned int hash, const char * str);
    JQ(int depth, unsigned int hash, jsmntok_t * tk, const char * js);
    ~JQ();

    bool operator==(struct jqpath &path);
    bool operator==(int val);
    bool operator==(float val);
    bool operator==(bool val);
    bool operator==(const char * str);
    bool operator==(std::string str);

    unsigned int get_depth() {return depth;}
    unsigned int get_hash() {return hash;}
    jsmntok_t * get_token() {return tk;}
    bool is_rendered() {return rendered;}


protected:
    void render_number();

private:
    unsigned int hash;      //! The hash of the path
    unsigned int depth;     //! The depth of the path
    bool rendered;          //! Has this value been redered to an actual value yet
    span sp;                //! Place in the string for the value
    jqvalue_u val;          //! The value
    jqvaltype_e type;       //! The value type
    jsmntok_t * tk;         //! Token ascociated with this value
    const char * js;        //! JSON string - cludge! 
};

#endif // !JQ_PATH
