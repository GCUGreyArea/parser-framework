#ifndef JSON_OUT_HEADER_H
#define JSON_OUT_HEADER_H

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>
#include "Output.h"
#include "Framework/Framework.h"

class JSONOut : public Output {
public:
    JSONOut()
        : mIdx(0) {}

    virtual void add_rule_value(Rule& r) {
        Pattern *p = r.matched();
        if(p != nullptr) {
            mOut["parsed"][mIdx]["rule"]["name"]        = r.name();
            mOut["parsed"][mIdx]["rule"]["id"]          = r.id();
            mOut["parsed"][mIdx]["event"]["name"]       = p->event();
            mOut["parsed"][mIdx]["event"]["pattern_id"] = p->id();
            std::vector<std::shared_ptr<Token>> tokens = p->get_tokens();
            for(auto& t : tokens) {
                switch(t->type().type()) {
                    default:
                    case ValueType::STRING:
                        mOut["parsed"][mIdx]["tokens"][t->name()] = t->as_string();
                        break;
                    case ValueType::INT:
                        mOut["parsed"][mIdx]["tokens"][t->name()] = t->as_int();
                        break;
                }
            }

            std::vector<ResolvedProperty> properties = p->get_properties();
            for(auto& p : properties) {
                mOut["parsed"][mIdx]["property"][p.name] = p.value;
            }

        }
        else {
            mOut["parsed"][mIdx]["rule"] = "null";
        }

        mIdx++;
    }

    virtual void add_rule_desc(Rule& t) {};
    virtual void add_pattern_value(Pattern& t) {};
    virtual void add_pattern_desc(Pattern& t) {};
    virtual void add_token_value(Token& t) {};
    virtual void add_token_desc(Token& t) {};

    friend std::ostream& operator<<(std::ostream& os, const JSONOut& obj) {
        os << obj.mOut;
        return os;
    }

private:
    int mIdx;
    nlohmann::json mOut;
};

#endif//JSON_OUT_HEADER_H
