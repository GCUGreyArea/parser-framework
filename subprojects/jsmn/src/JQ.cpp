
#include <cstring>
#include <string>
#include <exception>
#include <stdexcept>


#include "JQ.hpp"
#include "jsmn.hpp"

JQ::JQ()
    : hash(0), 
      depth(0), 
      rendered(false), 
      sp{0, 0}, 
      type(JQ_NO_VAL),
      tk(nullptr),
      js(nullptr) {}

JQ::JQ(struct jqpath &val)
    : hash(val.hash), 
      depth(val.depth), 
      rendered(val.rendered),
      sp(val.sp), 
      type(val.value.type), 
      tk(nullptr),
      js(nullptr)
{
    if (rendered) {
        switch (type) {
        case JQ_NO_VAL:
            break;
        case JQ_INT_VAL:
            this->val.int_val = val.value.value.int_val;
            break;
        case JQ_FLOAT_VAL:
            this->val.float_val = val.value.value.float_val;
            break;
        case JQ_STRING_VAL:
            this->val.string_val = copy_string(val.value.value.string_val);
            break;
        case JQ_BOOL_VAL:
            this->val.bool_val = val.value.value.bool_val;
            break;
        }
    }
}

JQ::JQ(int depth, unsigned int hash, span sp)
    : hash(hash),
      depth(depth),  
      rendered(false),
      sp(sp), 
      type(JQ_NO_VAL),
      tk(nullptr),
      js(nullptr) {}

JQ::JQ(int depth, unsigned int hash, const char *str)
    : hash(hash), 
      depth(depth), 
      rendered(true), 
      sp({0,0}),
      type(JQ_STRING_VAL),
      tk(nullptr),
      js(nullptr) 
{
    val.string_val = copy_string((char *)str);
}

JQ::JQ(int depth, unsigned int hash, jsmntok_t * tk, const char * js)
    : hash(hash),
      depth(depth),  
      rendered(false), 
      type(JQ_NO_VAL),
      tk(tk),
      js(js) {}

JQ::~JQ() {
    if (type == JQ_STRING_VAL && val.string_val != nullptr) {
        kill_string(val.string_val);
    }
}

bool JQ::operator==(struct jqpath &path) {
    if(path.op != JQ_EQUALS) {
        return false;
    }

    if(path.hash != hash || path.depth != depth) {
        return false;
    }

    if (path.value.type != JQ_NO_VAL && path.value.type == type) {
        switch (path.value.type) {
        case JQ_BOOL_VAL:
            return path.value.value.bool_val == val.bool_val;
        case JQ_INT_VAL:
            return path.value.value.int_val == val.int_val;
        case JQ_FLOAT_VAL:
            return path.value.value.float_val == val.float_val;
        case JQ_STRING_VAL:
            return cmp_string(path.value.value.string_val, val.string_val);
        default:
            break;
        }
    }

    return false;
}

bool JQ::operator==(int val) {
    if (this->type == JQ_INT_VAL) {
        return this->val.int_val == val;
    }

    
    return false;
}

bool JQ::operator==(float val) {
    if (this->type == JQ_FLOAT_VAL) {
        return this->val.float_val == val;
    }

    return false;
}

bool JQ::operator==(bool val) {
    if (this->type == JQ_BOOL_VAL) {
        return this->val.bool_val == val;
    }

    return false;
}

bool JQ::operator==(const char *str) {
    if(type == JQ_NO_VAL) {
        // Render the token
        if(rendered == false && tk != nullptr) {
            if(this->js == nullptr) {
                throw std::runtime_error("JSON string not set but token non null pointer");
            }

            // Render the token
            switch(tk->type) {
                case JSMN_STRING: {
                    // Include the quotes in the captured string
                    int len = this->tk->end - this->tk->start + 2;
                    type = JQ_STRING_VAL;
                    val.string_val = (char*)malloc(len+1);
                    if(val.string_val == nullptr) {
                        throw std::runtime_error("memory allcoation failure");
                    }

                    memccpy(val.string_val,&js[tk->start-1],1,len);
                    val.string_val[len] = '\0';

                    return *this == str;

                }
                break;
                
                case JSMN_PRIMITIVE:
                    return false;

                case JSMN_ARRAY: {
                    jsmntok_t * tkn = tk+1; 
                    int len = strlen(str);
                    for(int i=0;i<tk->size;i++) {
                        if(tkn[i].type == JSMN_STRING) {
                            int len1 = (tkn[i].end+1) - (tkn[i].start-1);
                            if(len1 == len) {
                                if(strncmp(&js[tkn[i].start-1],str,len) == 0) {
                                    return true;
                                }
                            }
                       }
                    }
                }
                break;
                case JSMN_OBJECT:
                    throw std::runtime_error("unsupported comparison");  

                case JSMN_UNDEFINED:
                    throw std::runtime_error("undefined value");
            }
        }
    }
    else if (this->type == JQ_STRING_VAL) {
        return std::strcmp(this->val.string_val, str) == 0;
    }

    return false;
}

bool JQ::operator==(std::string str) {
    if (this->type == JQ_STRING_VAL) {
        return str == this->val.string_val;
    }

    return false;
}

void JQ::render_number() {
    int idx = 0;
    bool fl = false;
    while(js[tk->start+idx] != '\0') {
        if(js[tk->start+idx] == '.') {
            fl = true;
            goto done;
        }
    }

done:
    int len = tk->end - tk->start;
    char * str = new char[len+1];
    if(str == nullptr) {
        throw std::runtime_error("memory allocation failure");
    }

    memccpy(str,&js[tk->start],1,len);
    str[len] = '\0';
    if(fl) {
        type = JQ_FLOAT_VAL;
        val.float_val = atof(str);
    }
    else {
        type = JQ_INT_VAL;
        val.int_val = atoi(str);
    }

    delete [] str;
}
