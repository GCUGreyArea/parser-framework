#ifndef DYNAMIC_PROPERTY_AST_H
#define DYNAMIC_PROPERTY_AST_H

#include <memory>
#include <string>
#include <vector>

namespace DynamicProperty {

enum class ArithmeticOperator {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE
};

enum class ComparisonOperator {
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    LESS_OR_EQUAL,
    GREATER_THAN,
    GREATER_OR_EQUAL,
    CONTAINS
};

enum class LogicalOperator {
    AND,
    OR
};

struct ValueExpression;
struct BooleanExpression;

using ValueExpressionPtr = std::shared_ptr<ValueExpression>;
using BooleanExpressionPtr = std::shared_ptr<BooleanExpression>;

struct ValueExpression {
    virtual ~ValueExpression() = default;
};

struct IdentifierExpression : ValueExpression {
    explicit IdentifierExpression(std::string name)
        : name(std::move(name)) {}

    std::string name;
};

struct StringLiteralExpression : ValueExpression {
    explicit StringLiteralExpression(std::string value)
        : value(std::move(value)) {}

    std::string value;
};

struct NumberLiteralExpression : ValueExpression {
    explicit NumberLiteralExpression(int value)
        : value(value) {}

    int value;
};

struct BinaryValueExpression : ValueExpression {
    BinaryValueExpression(ArithmeticOperator op, ValueExpressionPtr lhs, ValueExpressionPtr rhs)
        : op(op)
        , lhs(std::move(lhs))
        , rhs(std::move(rhs)) {}

    ArithmeticOperator op;
    ValueExpressionPtr lhs;
    ValueExpressionPtr rhs;
};

struct BooleanExpression {
    virtual ~BooleanExpression() = default;
};

struct ComparisonExpression : BooleanExpression {
    ComparisonExpression(ComparisonOperator op, ValueExpressionPtr lhs, ValueExpressionPtr rhs)
        : op(op)
        , lhs(std::move(lhs))
        , rhs(std::move(rhs)) {}

    ComparisonOperator op;
    ValueExpressionPtr lhs;
    ValueExpressionPtr rhs;
};

struct LogicalExpression : BooleanExpression {
    LogicalExpression(LogicalOperator op, BooleanExpressionPtr lhs, BooleanExpressionPtr rhs)
        : op(op)
        , lhs(std::move(lhs))
        , rhs(std::move(rhs)) {}

    LogicalOperator op;
    BooleanExpressionPtr lhs;
    BooleanExpressionPtr rhs;
};

struct AssignmentStatement {
    AssignmentStatement() = default;

    AssignmentStatement(std::string target, ValueExpressionPtr expr)
        : target(std::move(target))
        , expr(std::move(expr)) {}

    std::string target;
    ValueExpressionPtr expr;
};

struct ConditionalAst {
    ConditionalAst(BooleanExpressionPtr condition, std::vector<AssignmentStatement> then_assignments, std::vector<AssignmentStatement> else_assignments)
        : condition(std::move(condition))
        , then_assignments(std::move(then_assignments))
        , else_assignments(std::move(else_assignments)) {}

    BooleanExpressionPtr condition;
    std::vector<AssignmentStatement> then_assignments;
    std::vector<AssignmentStatement> else_assignments;
};

using ConditionalAstPtr = std::shared_ptr<ConditionalAst>;

} // namespace DynamicProperty

#endif//DYNAMIC_PROPERTY_AST_H
