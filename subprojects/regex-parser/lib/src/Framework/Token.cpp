#include "BaseValue.h"
#include "Token.h"

#include "Exception/Exceptions.h"


Token::Token(std::string tokenName,std::string regexVal,ValueType type, size_t idx)
    : mName(tokenName)
    , mRegex(regexVal)
    , mType(type)
    , mIdx(idx)
    , mMatched(false)
    , mMatchNum(0) {}

Token::Token(const Token& tk)
    : mName(tk.mName)
    , mRegex(tk.mRegex)
    , mType(tk.mType)
    , mIdx(tk.mIdx)
    , mMatched(tk.mMatched)
    , mMatchNum(0) {}

std::string Token::name() {
    return mName;
}


ValueType Token::type() const {
    return mType;
}

/**
 * @brief A token matches an input string if the match can be found in the string
 *
 * @param msg
 * @return true
 * @return false
 */
bool Token::match(Match& m) {
    // We might be retesting the same token
    if(mMatched)
        return true;

    mMatched = mRegex.match(m);

    return mMatched;
}

bool Token::match(Match& m, std::string assert) {
    mMatched = mRegex.match(m,assert);

    return mMatched;
}

bool Token::match_raw(Match& m) {
    // We might be retesting the same token
    mMatched = mRegex.match(m);

    return mMatched;
}

void Token::print() {
    // std::cout << "Token [name " << mName << " / regex " << mRegexVal << "]" << std::endl;
}

void Token::validate() {
    if(mName == "")
        throw Exception::NoName("Token");

    mRegex.validate();
}

std::string Token::as_string() const {
    return mRegex.as_string();
}

int Token::as_int() const {
    return mRegex.as_int();
}

bool Token::operator==(const Token &other) const {
    if(mType != other.mType)
        return false;


    if(mType == ValueType::INT) {
        return as_int() == other.as_int();
    }

    return as_string() == other.as_string();
}

bool Token::operator!=(const Token &other) const {
    return !(*this == other);
}

void Token::reset() {
    mMatched=false;
    mMatchNum=0;
}

bool Token::operator<(const Token &other) const {
    if(mType != other.mType)
        return false;

    if(mType == ValueType::INT) {
        return as_int() < other.as_int();
    }

    return as_string() < other.as_string();
}

bool Token::operator>(const Token &other) const {
    return other < *this;
}

bool Token::operator<=(const Token &other) const {
    return !(other < *this);
}

bool Token::operator>=(const Token &other) const {
    return !(*this < other);
}
