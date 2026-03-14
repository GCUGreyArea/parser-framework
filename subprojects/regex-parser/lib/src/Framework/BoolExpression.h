#ifndef EXPRESSION_H
#define EXPRESSION_H
/**
 * @file BoolExpression.h
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief An expression as a tree of expressions.
 * Each leaf (lhs or rhs) can either be a pointer to another BoolExpression or an C
 * @version 1.0
 * @date 27-04-2022
 *
 * @copyright Copyright (C) Barry Robinson 2022
 *
 */

#include <vector>
#include "Condition.h"
#include "Exception/Exceptions.h"

class BoolExpression {
public:
    enum Conjunction {
        NONE,
        AND,
        OR
    };


    static Conjunction from_string(std::string op) {
        if(op == "and") return AND;
        if(op == "or")  return OR;

        return NONE;
    }

    BoolExpression()
        : mConj(NONE)
        , mLhs()
        , mRhs()
        , mLhsExp(nullptr)
        , mRhsExp(nullptr) {}

    BoolExpression(Condition& lhs)
        : mConj(NONE)
        , mLhs(lhs)
        , mRhs()
        , mLhsExp(nullptr)
        , mRhsExp(nullptr) {}

    BoolExpression(Condition& lhs, Conjunction conj, Condition& rhs)
        : mConj(conj)
        , mLhs(lhs)
        , mRhs(rhs)
        , mLhsExp(nullptr)
        , mRhsExp(nullptr) {}

    BoolExpression(std::shared_ptr<BoolExpression> lhs, Conjunction conj, Condition& rhs)
        : mConj(conj)
        , mLhs()
        , mRhs(rhs)
        , mLhsExp(lhs)
        , mRhsExp(nullptr) {}

    BoolExpression(Condition& lhs, Conjunction conj, std::shared_ptr<BoolExpression> rhs)
        : mConj(conj)
        , mLhs(lhs)
        , mRhs()
        , mLhsExp(nullptr)
        , mRhsExp(rhs) {}

    BoolExpression(std::shared_ptr<BoolExpression> lhs, Conjunction conj, std::shared_ptr<BoolExpression> rhs)
        : mConj(conj)
        , mLhs()
        , mRhs()
        , mLhsExp(lhs)
        , mRhsExp(rhs) {}

    /**
     * @brief Recursively ev aluate the tree
     *
     * @return true
     * @return false
     */
    bool evaluate() {
        bool res = false;
        if(mLhsExp)
            res = mLhsExp->evaluate();
        else
            res = mLhs.evaluate();

        if(mConj == NONE)
            return res;

        if(mConj == AND)
            // OR will try rhs, AND will fail
            if(!res)
                return res;
        if(mConj == OR)
            if(res)
                return res;

        if(mRhsExp)
            res = mRhsExp->evaluate();
        else
            res = mRhs.evaluate();

        return res;
    }


    bool valid() {
        // If there is a conjunction there needs to be a valid lhs and rhs
        if(mConj != NONE)
            if((mLhs.valid() || mLhsExp != nullptr) && (mRhs.valid() || mRhsExp != nullptr))
                return true;

        // If there isn't we only need one valid Condition
        if(mLhs.valid())
            return true;

        return false;
    }


    bool valid_rhs() {
        if(mConj != NONE && (mRhs.valid() || mRhsExp != nullptr))
            return true;
        return false;
    }


    bool valid_conj() {
        if(mConj != NONE)
            return true;
        return false;
    }


    bool complete() {
        if(mConj != NONE)
            if((mLhs.valid() || mLhsExp != nullptr) && (mRhs.valid() || mRhsExp != nullptr))
                return true;

        return false;
    }

    bool lhs_only() {
        if(mConj == NONE)
            if(mLhs.valid() || mLhsExp != nullptr)
                return true;

        return false;
    }

    bool empty() {
        if(mConj == NONE && !mLhs.valid() && mLhsExp == nullptr && !mRhs.valid() && mRhsExp == nullptr)
            return true;

        return false;
    }

    void set_rhs(Condition& rhs, Conjunction op) {
        mConj = op;
        mRhs = rhs;
    }

    void set_rhs(std::shared_ptr<BoolExpression> rhs, Conjunction op) {
        mConj = op;
        mRhsExp = rhs;
    }

    void set_lhs(Condition& lhs) {
        mLhs = lhs;
    }

private:
    Conjunction mConj;

    //! Right and left hand side of the expression
    Condition mLhs;
    Condition mRhs;

    //! Can also be a complex expression in itself
    std::shared_ptr<BoolExpression> mLhsExp;
    std::shared_ptr<BoolExpression> mRhsExp;

};

#endif//EXPRESSION_H
