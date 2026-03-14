#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

/**
 * @file Assignment.h
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief This is a simple assignment operation that does not deal with the notion of
 * operator precedence or anything more complex than more than concatenation of string or
 * the addition and subtraction of numbers. This is due to
 * 1. Addition and asubtraction are commutative and associative in that
 *    (3 + 2) - 4 == 3 + (2 - 4) == 1
 * If we add in multiplication we no longer have the same ordering
 *
 *    (3 * 2) - 4 != 3* (2 - 4)
 * 2. The relatively simple representation of an expression
 * @version 1.0
 * @date 08-05-2022
 *
 * @copyright Copyright (C) Barry Robinson 2022
 *
 */

#include <memory>
#include <vector>
#include "Variable.h"

class Assignment {
public:
    enum Operator {
        EQUALS,
        PLUS,
        MINUS
    };


    static Operator from_string(std::string op) {
        if(op == "=") return EQUALS;
        if(op == "+") return PLUS;
        if(op == "-") return MINUS;

        throw Exception::General("Unknown operator \"" + op + "\"");
    }

    static std::string op_to_string(Operator op) {
        switch(op) {
            case EQUALS:    return "equals";
            case PLUS:      return "plus";
            case MINUS:     return "minus";

            default:
                break;
        }

        return "unknown";
    }

    Assignment(std::shared_ptr<Variable> lhs, Operator op, std::shared_ptr<Variable> rhs)
        : mLhs(lhs)
        , mOp(op)
        , mRhs(rhs)
        , mRhsTree(nullptr) {}

    Assignment(std::shared_ptr<Variable> lhs, Operator op, std::shared_ptr<Assignment> rhs)
        : mLhs(lhs)
        , mOp(op)
        , mRhs()
        , mRhsTree(rhs) {}

    // TODO: Test that a variable can be reassigned.
    // TODO: Make this non recursive!
    std::shared_ptr<Variable> evaluate() {
        switch(mOp) {
            case EQUALS: {
                if(mRhsTree)
                    *mLhs = *(mRhsTree->evaluate());
                else
                    *mLhs = *mRhs;
                return mLhs;
            }
            case PLUS: {
                if(mRhsTree)
                    return std::make_shared<Variable>(*mLhs + *(mRhsTree->evaluate()));

                return std::make_shared<Variable>(*mLhs + *mRhs);
            }
            case MINUS: {
                if(mRhsTree)
                    return std::make_shared<Variable>(*mLhs - *(mRhsTree->evaluate()));

                return std::make_shared<Variable>(*mLhs - *mRhs);
            }
        }

        return nullptr;
    }

    std::shared_ptr<Variable> get_assignmet_variable() {
        if(mOp != EQUALS)
            throw Exception::General("Lhs assignment variable uses " + op_to_string(mOp) + " operator so is not part of an assignment expression");


        return mLhs;
    }

    static std::shared_ptr<Assignment> build(std::vector<std::shared_ptr<Variable>>& var, std::vector<Assignment::Operator>& ops);

private:
    std::shared_ptr<Variable> mLhs;
    Operator mOp;
    std::shared_ptr<Variable> mRhs;
    std::shared_ptr<Assignment> mRhsTree;
};
#endif//ASSIGNMENT_H