#ifndef CONDITION_H
#define CONDITION_H

#include <memory>
#include <iostream>

#include "Token.h"
#include "BaseValue.h"

#include "Exception/InvalidOperator.h"

class Condition {
public:
    enum Operator {
        INVALID,
        EQUAL_TO,
        LESS_THAN,
        GREATER_THAN,
        GREATER_OR_EQUAL,
        LESS_OR_EQUAL,
        NOT_EQUAL,
        STR_CONTAINS
    };

    Condition()
        : mLhs()
        , mOp(INVALID)
        , mRhs() {}

    Condition(BaseValue& v1, Operator op, BaseValue& v2)
        : mLhs(v1)
        , mOp(op)
        , mRhs(v2) {}

    Condition(BaseValue& v1, std::string op, BaseValue& v2)
        : mLhs(v1)
        , mOp(to_operator(op))
        , mRhs(v2) {
            if(mOp == INVALID) throw Exception::InvalidOperator(op);
        }

    Condition(const Condition& other)
        : mLhs(other.mLhs)
        , mOp(other.mOp)
        , mRhs(other.mRhs) {}

    static Operator to_operator(std::string op) {
        if(op == "==") return EQUAL_TO;
        if(op == "<")  return LESS_THAN;
        if(op == ">")  return GREATER_THAN;
        if(op == ">=") return GREATER_OR_EQUAL;
        if(op == "<=") return LESS_OR_EQUAL;
        if(op == "!=") return NOT_EQUAL;
        if(op == "?=") return STR_CONTAINS;

        return INVALID;
    }

    bool evaluate() {
        switch(mOp) {
            case EQUAL_TO:         return  mLhs == mRhs;
            case LESS_THAN:        return  mLhs <  mRhs;
            case GREATER_THAN:     return  mLhs >  mRhs;
            case GREATER_OR_EQUAL: return  mLhs >= mRhs;
            case LESS_OR_EQUAL:    return  mLhs <= mRhs;
            case NOT_EQUAL:        return  mLhs != mRhs;
            case STR_CONTAINS: {
                if(mRhs.type() == ValueType::STRING && mLhs.type() == ValueType::STRING)
                    return mLhs.as_string().find(mRhs.as_string()) != std::string::npos;

                return false;
            }

            default:
                std::cerr << "Invalid enum value in Condition operator [value " << mOp << "]" << std::endl;
                abort();
        }

        return false;
    }

    void operator=(const Condition& other) {
        mLhs = other.mLhs;
        mOp = other.mOp;
        mRhs = other.mRhs;
    }

    void set_lhs(std::shared_ptr<Token> v) {
        mLhs.set(v);
    }

    void set_lhs(std::string v) {
        mLhs.set(v);
    }

    void set_lhs(int v) {
        mLhs.set(v);
    }

    void set_rhs(std::shared_ptr<Token> v) {
        mRhs.set(v);
    }

    void set_rhs(std::string v) {
        mRhs.set(v);
    }

    void set_rhs(int v) {
        mRhs.set(v);
    }

    bool valid() {
        if(mOp != INVALID)
            if(mLhs.valid() && mRhs.valid()) {
                return true;
            }

        return false;
    }

    void clear() {
        mLhs.clear();
        mOp = INVALID;
        mRhs.clear();
    }

private:
    BaseValue mLhs;
    Operator mOp;
    BaseValue mRhs;
};

#endif//CONDITION_H
