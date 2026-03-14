#ifndef VARIABLE_H
#define VARIABLE_H


#include <iostream>
#include "Exception/Exceptions.h"
#include "BaseValue.h"


class Variable {
public:
    Variable(std::string name, ValueType type, bool cnst = false)
        : mName(name)
        , mValue(type)
        , mConst(cnst) {}

    Variable(std::string name, size_t val, bool cnst = false)
        : mName(name)
        , mValue(val)
        , mConst(cnst) {}

    Variable(std::string name, std::string val, bool cnst = false)
        : mName(name)
        , mValue(val)
        , mConst(cnst) {}

    Variable(std::string name, std::shared_ptr<Token> val, bool cnst = true)
        : mName(name)
        , mValue(val)
        , mConst(cnst) {}

    Variable(const Variable& v)
        : mName(v.mName)
        , mValue(v.mValue)
        , mConst(v.mConst) {}


    Variable(std::string name, const BaseValue& v, bool cnts = false)
        : mName(name)
        , mValue(v)
        , mConst(cnts) {}

    Variable()
        : mName("anonymous")
        , mValue()
        , mConst(false) {}

    bool operator==(const Variable& v2) const {
        return mValue == v2.mValue;
    }

    bool operator>(const Variable& v2) const {
        return mValue > v2.mValue;
    }

    bool operator<(const Variable& v2) const {
        return mValue < v2.mValue;
    }

    bool operator<=(const Variable& v) const {
       return mValue <= v.mValue;
    }

    bool operator>=(const Variable& v) const {
        return mValue >= v.mValue;
    }

    void operator=(const Variable& v) {
        if(mConst)
            throw Exception::CantSetConstVariable(mName);

        mValue = v.mValue;
    }

    friend Variable operator+(const Variable& v1, const Variable& v2) {
        Variable v{"tmp", v1.mValue + v2.mValue};
        return v;
    }

    friend Variable operator-(const Variable& v1, const Variable& v2) {
        Variable v("tmp", v1.mValue - v2.mValue);
        return v;
    }


    friend Variable operator/(const Variable& v1, const Variable& v2) {
        Variable v("tmp", v1.mValue / v2.mValue);
        return v;
    }

    std::string as_string() {
        return mValue.as_string();
    }
    size_t as_int() {return mValue.as_int();}
    std::string name() {return mName;}


    void print() {
        std::cout << "Name " << mName << std::endl;
        std::cout << "Value " << mValue << std::endl;
    }

    bool const_var() {return mConst;}

private:
    std::string mName;
    BaseValue mValue;
    bool mConst;
};

#endif//VARIABLE_H
