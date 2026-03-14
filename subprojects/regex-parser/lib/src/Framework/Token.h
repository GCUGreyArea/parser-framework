#ifndef TOKEN_H
#define TOKEN_H

/**
 * @file Token.h
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief Represent a token with its matching regex
 * @version 1.0
 * @date 12-03-2022
 *
 * @copyright Copyright (C) Barry Robinson 2022
 *
 */
#include <string>
#include <iostream>
#include <memory>
#include <regex>

#include "Regex.h"
#include "ValueType.h"

class BaseValue;

class Token {
public:
    friend class BaseValue;

    Token(std::string tokenName, std::string regexVal, ValueType type, size_t idx);
    Token(const Token& tk);

    std::string name();
    ValueType type() const;
    bool match(Match& m);
    bool match(Match& m, std::string assert);
    bool match_raw(Match& m);
    void print();

    void validate();
    std::string as_string() const;
    int as_int() const;

    bool matched() {return mMatched;}
    void set_match_number(size_t nth) {mMatchNum=nth;}
    size_t get_match_num() {return mMatchNum;}
    void reset();

    bool contains(std::string str) {
        std::string tk = mRegex.as_string();
        return tk.find(str) != std::string::npos;
    }

    bool operator==(const Token &other) const;
    bool operator!=(const Token &other) const;

    bool operator<(const Token &other) const;
    bool operator>(const Token &other) const;
    bool operator<=(const Token &other) const;
    bool operator>=(const Token &other) const;


private:
    std::string mName;
    Regex mRegex;
    ValueType mType;
    size_t mIdx;
    bool mMatched;
    size_t mMatchNum;
};


#endif // TOKEN_H
