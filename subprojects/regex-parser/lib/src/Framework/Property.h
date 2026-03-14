#ifndef PROPERTY_H
#define PROPERTY_H

#include <memory>
#include <string>
#include <vector>

#include "ResolvedProperty.h"
#include "Token.h"

class Property {
public:
    Property(std::vector<std::shared_ptr<Token>>& tokens, std::string name, std::string delim)
        : mTokens(tokens)
        , mName(name)
        , mDelim(delim) {}

    std::string name() const {return mName;}

    ResolvedProperty resolve() const {
        std::string value;
        size_t size = mTokens.size();
        for(size_t i=0;i<size;i++) {
            const auto& t = mTokens.at(i);
            value.append(t->as_string());
            if(i+1<size)
                value.append(mDelim);
        }

        return {mName, value};
    }

private:
    std::vector<std::shared_ptr<Token>> mTokens;
    std::string mName;
    std::string mDelim;
};


#endif // PROPERTY_H
