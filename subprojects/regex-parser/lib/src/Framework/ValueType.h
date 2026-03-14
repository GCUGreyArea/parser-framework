#ifndef VALUE_TYPE_H
#define VALUE_TYPE_H
/**
 * @file ValueType.h
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief Value type for variables and tokens, etc
 * @version 1.0
 * @date 29-04-2022
 *
 * @copyright Copyright (C) Barry Robinson 2022
 *
 */

#include <string>

class ValueType {
public:
    enum Type {
        NONE,
        INT,
        STRING
    };

    ValueType(std::string s)
        : mType(from_string(s)) {}

    ValueType(Type t)
        : mType(t) {}

    ValueType(const ValueType& t)
        : mType(t.mType) {}

    Type from_string(std::string s) {
        if(s == "int")          return INT;
        else if(s == "string")  return STRING;

        return NONE;
    }

    std::string to_string(ValueType t) {
        return to_string(t.mType);
    }

    std::string to_string() {
        return to_string(mType);
    }

    bool operator==(const ValueType &other) const;
    bool operator!=(const ValueType &other) const;

    bool operator==(const Type other) const {
        return mType == other;
    }

    bool operator!=(const Type other) const {
        return mType != other;
    }

    bool operator==(std::string t) {
        return mType == from_string(t);
    }

    Type type() const {return mType;}

    std::string type_string() {
        return to_string(mType);
    }


protected:
    std::string to_string(Type t) {
        switch(t) {
            case INT:    return "int";
            case STRING: return "string";
            default:
                break;
        }

        return "none";
    }
private:
    Type mType;
};

inline bool ValueType::operator==(const ValueType &other) const {
    return mType == other.mType;
}

inline bool ValueType::operator!=(const ValueType &other) const {
    return !(*this == other);
}

#endif//VALUE_TYPE_H