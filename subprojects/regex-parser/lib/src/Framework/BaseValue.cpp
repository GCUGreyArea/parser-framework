#include "Token.h"
#include "BaseValue.h"
#include "ValueType.h"
#include "Exception/Exceptions.h"

/**
 * @brief Equivalence for a BaseValue depends on the two types and
 * if one value is a token
 *
 * @param other
 * @return true
 * @return false
 */
bool BaseValue::operator==(const BaseValue &other) const
{
    // At this stage bar comparision of different types
    if (mType != other.mType)
        return false;

    if(other.mToken) {
        if(mToken)
            return *mToken == *other.mToken;

        if(mType == ValueType::STRING)
            return mStr == other.mToken->as_string();
        else if(mType == ValueType::INT)
            return mInt == other.mToken->as_int();
    }

    if(mToken) {
        if(other.mType == ValueType::INT)
            return mToken->as_int() == other.mInt;
        else if(other.mType == ValueType::STRING)
            return mToken->as_string() == other.mStr;
    }

    // We are now logically left with both BaseTypes being native types, and the same type!
    if (mType == ValueType::INT)
        return mInt == other.mInt;
    else if(mType == ValueType::STRING)
        return mStr == other.mStr;

    // Make the compiler and guard against disaster? Maybe...
    return false;
}

bool BaseValue::operator!=(const BaseValue &other) const {
    return !(*this == other);
}

bool BaseValue::operator<(const BaseValue &other) const {
    if (mType != other.mType)
        return false;

    if(other.mToken) {
         if(mToken)
            return *mToken < *other.mToken;

        if(mType == ValueType::STRING)
            return mStr < other.mToken->as_string();
        else
            return mInt < other.mToken->as_int();
    }
    else if(mToken) {
        if(other.mType == ValueType::STRING)
            return mToken->as_string() < other.mStr;
        else
            return mToken->as_int() < other.mInt;
    }

    if (mType == ValueType::INT)
        return mInt < other.mInt;

    return mStr < other.mStr;
}

bool BaseValue::operator>(const BaseValue &other) const {
    return other < *this;
}

bool BaseValue::operator<=(const BaseValue &other) const {
    return !(other < *this);
}

bool BaseValue::operator>=(const BaseValue &other) const {
    return !(*this < other);
}

void BaseValue::operator=(const BaseValue &other) {
    if (mToken)
        throw Exception::CantSetConstVariable("Token is implicitly a constant value in BaseValue [token " + mToken->mName + "]");

    // This is an unasigned value
    if(mType == ValueType::NONE) {
        mType = other.mType;
    }

    if(mType != other.mType) {
        if (mType == ValueType::STRING && other.mType == ValueType::INT) {
            mStr = std::to_string(other.mInt);
        }
        else if(mType == ValueType::INT && other.mType == ValueType::STRING) {
            std::string str = other.as_string();
            unsigned int base = Numbers::is_number(str);
            if(base > 0) {
                mInt = Numbers::to_int(str,base);
            }
        }
    }
    else {
        if (other.mToken)
            mToken = other.mToken;
        else if(other.mType == ValueType::INT)
            mInt = other.mInt;
        else if(other.mType == ValueType::STRING)
            mStr = other.mStr;
        else
            throw Exception::General("Attempting to set BaseValue to uninitialized BaseValue");
    }
}

BaseValue operator+(const BaseValue& v1, const BaseValue& v2) {
    if(v1.mType == ValueType::INT && v2.mType == ValueType::INT)
        return {v1.as_int() + v2.as_int()};

    return {v1.as_string() + v2.as_string()};
}

BaseValue operator-(const BaseValue& v1, const BaseValue& v2) {
    if(v1.mType == ValueType::INT)
        return {v1.as_int() - v2.as_int()};

    return {};

}

BaseValue operator/(const BaseValue& v1, const BaseValue& v2) {
    if(v1.mType == ValueType::INT)
        return {v1.as_int() / v2.as_int()};

    return {};

}

BaseValue operator*(const BaseValue& v1, const BaseValue& v2) {
    if(v1.mType == ValueType::INT)
        return {v1.as_int() * v2.as_int()};

    return {};

}


/**
 * @brief Determin if the literal is a valid number
 *
 * @param s
 * @return unsigned int
 */

unsigned int BaseValue::is_number(std::string s)
{
    const char *str = s.data();

    unsigned int ret = 10;

    if (*str == '0')
    {
        str++;
        if (*str == 'x' || *str == 'X')
        {
            ret = 16;
            str++;
        }
        else if (*str == 'b' || *str == 'B')
        {
            ret = 2;
            str++;
        }
    }

    while (*str != '\0')
    {
        switch (*str)
        {
        case '0' ... '1':
            break;
        case '2' ... '9':
            if (ret == 2)
                return 0;
            break;

        case 'A' ... 'F':
        case 'a' ... 'f':
            if (ret != 16)
                return 0;
            break;

        default:
            return 0;
        }
        str++;
    }

    return true;
}

int BaseValue::as_int() const {
    if(mToken)
        return mToken->as_int();

    return mInt;
}

std::string BaseValue::as_string() const {
    if(mToken)
        return mToken->as_string();

    if(mType == ValueType::INT)
        return std::to_string(mInt);

    return mStr;
}

BaseValue::BaseValue(std::shared_ptr<Token> t)
    : mToken(t)
    , mStr("")
    , mInt(-1)
    , mType(t->type()) {}


std::ostream &operator<<(std::ostream &os, const BaseValue &rhs) {
    if(rhs.mToken)
        os << "mToken: " << rhs.mToken->as_string() << std::endl;
    else if(rhs.mType == ValueType::STRING)
        os << "mStr: " << rhs.mStr << std::endl;
    else if(rhs.mType == ValueType::INT)
        os << " mInt: " << rhs.mInt << std::endl;

    return os;
}
