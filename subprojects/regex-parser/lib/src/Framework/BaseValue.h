#ifndef BASE_VALUE_H
#define BASE_VALUE_H

#include <cstddef>
#include <string>
#include <ostream>
#include <iostream>
#include <memory>

#include "ValueType.h"

class Token;

class BaseValue {
public:
    friend class Token;

    BaseValue(int v)
        : mToken(nullptr)
        , mStr("")
        , mInt(v)
        , mType(ValueType::INT) {}

    BaseValue(std::string v)
        : mToken(nullptr)
        , mStr(v)
        , mInt(0)
        , mType(ValueType::STRING) {}

    BaseValue(std::shared_ptr<Token> t);

    BaseValue(ValueType type)
        : mToken(nullptr)
        , mStr("")
        , mInt(0)
        , mType(type) {}

    BaseValue()
        : mType(ValueType::NONE) {}

    BaseValue(const BaseValue& other)
        : mToken(other.mToken)
        , mStr(other.mStr)
        , mInt(other.mInt)
        , mType(other.mType) {}

    void set(std::shared_ptr<Token> t) {
        if(mType != ValueType::NONE || mToken != nullptr) {
            std::cerr << "BaseValue has already been set" << std::endl;
            abort();
        }
        mToken = t;
    }

    void set(std::string s) {
        if(mType != ValueType::NONE || mToken != nullptr) {
            std::cerr << "BaseValue has already been set" << std::endl;
            abort();
        }
        mStr = s;
        mType = ValueType::STRING;
    }

    void set(int v) {
        if(mType != ValueType::NONE || mToken != nullptr) {
            std::cerr << "BaseValue has already been set" << std::endl;
            abort();
        }
        mInt = v;
        mType = ValueType::INT;
    }

    ValueType type() {return mType;}

    bool operator==(const BaseValue &other) const;
    bool operator!=(const BaseValue &other) const;
    bool operator<(const BaseValue &other) const;
    bool operator>(const BaseValue &other) const;
    bool operator<=(const BaseValue &other) const;
    bool operator>=(const BaseValue &other) const;
    void operator=(const BaseValue& other);

    friend BaseValue operator+(const BaseValue& v1, const BaseValue& v2);
    friend BaseValue operator-(const BaseValue& v1, const BaseValue& v2);
    friend BaseValue operator/(const BaseValue& v1, const BaseValue& v2);
    friend BaseValue operator*(const BaseValue& v1, const BaseValue& v2);


    bool valid() {
        if(mType == ValueType::NONE)
            return false;

        return true;
    }


    void set_type(ValueType type) {mType=type;}
    std::string as_string() const;
    int as_int() const;

    friend std::ostream &operator<<(std::ostream &os, const BaseValue &rhs);


    void clear() {
        mToken = nullptr;
        mType = ValueType::NONE;
    }


protected:
    unsigned int is_number(std::string s);
    void render_token();

private:
    std::shared_ptr<Token> mToken;
    std::string mStr;
    int mInt;
    ValueType mType;
};

#endif//BASE_VALUE_H