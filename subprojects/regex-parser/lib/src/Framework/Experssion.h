#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Variable.h"

class Expression {
public:
    enum Operator {
        EQUALS,
        GREATER,
        GREATER_OR_EQUALS,
        LESS,
        LESS_OR_EQUALS,
        PLUS,
        MINUS,
        TIMES,
        DIVIDE
    };

private:
    std::shared_ptr<Variable> mLhs;
    std::shared_ptr<Expression> mLhsTree;
    Operator mOp;
    std::shared_ptr<Variable> mRhs;
    std::shared_ptr<Expression> mRhsTree;


};

#endif//EXPRESSION_H