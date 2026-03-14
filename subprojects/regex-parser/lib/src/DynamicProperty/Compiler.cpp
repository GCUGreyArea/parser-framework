#include "Compiler.h"

#include <memory>
#include <utility>
#include <vector>

#include "DynamicPropertyParser.hpp"
#include "Exception/Exceptions.h"
#include "Framework/BaseValue.h"
#include "Framework/Condition.h"

typedef void * yyscan_t;
struct yy_buffer_state;
typedef yy_buffer_state *YY_BUFFER_STATE;
int yylex_init(yyscan_t * scanner);
int yylex_destroy(yyscan_t scanner);
YY_BUFFER_STATE yy_scan_string(const char * yy_str, yyscan_t yyscanner);
void yy_delete_buffer(YY_BUFFER_STATE buffer, yyscan_t yyscanner);

namespace DynamicProperty {

namespace {

class CompiledValueExpression {
public:
    virtual ~CompiledValueExpression() = default;
    virtual BaseValue evaluate() const = 0;
};

class TokenValueExpression : public CompiledValueExpression {
public:
    explicit TokenValueExpression(std::shared_ptr<Token> token)
        : mToken(std::move(token)) {}

    BaseValue evaluate() const override {
        return BaseValue(mToken);
    }

private:
    std::shared_ptr<Token> mToken;
};

class LiteralValueExpression : public CompiledValueExpression {
public:
    explicit LiteralValueExpression(BaseValue value)
        : mValue(std::move(value)) {}

    BaseValue evaluate() const override {
        return mValue;
    }

private:
    BaseValue mValue;
};

class BinaryCompiledValueExpression : public CompiledValueExpression {
public:
    BinaryCompiledValueExpression(ArithmeticOperator op, std::shared_ptr<CompiledValueExpression> lhs, std::shared_ptr<CompiledValueExpression> rhs)
        : mOp(op)
        , mLhs(std::move(lhs))
        , mRhs(std::move(rhs)) {}

    BaseValue evaluate() const override {
        BaseValue lhs = mLhs->evaluate();
        BaseValue rhs = mRhs->evaluate();

        switch(mOp) {
            case ArithmeticOperator::ADD:
                return lhs + rhs;
            case ArithmeticOperator::SUBTRACT:
                return lhs - rhs;
            case ArithmeticOperator::MULTIPLY:
                return lhs * rhs;
            case ArithmeticOperator::DIVIDE:
                return lhs / rhs;
        }

        throw Exception::General("Unknown arithmetic operator");
    }

private:
    ArithmeticOperator mOp;
    std::shared_ptr<CompiledValueExpression> mLhs;
    std::shared_ptr<CompiledValueExpression> mRhs;
};

class CompiledConditionEvaluator : public ConditionalAssignment::ConditionEvaluator {
public:
    virtual ~CompiledConditionEvaluator() = default;
};

class ComparisonConditionEvaluator : public CompiledConditionEvaluator {
public:
    ComparisonConditionEvaluator(ComparisonOperator op, std::shared_ptr<CompiledValueExpression> lhs, std::shared_ptr<CompiledValueExpression> rhs)
        : mOp(op)
        , mLhs(std::move(lhs))
        , mRhs(std::move(rhs)) {}

    bool evaluate() override {
        BaseValue lhs = mLhs->evaluate();
        BaseValue rhs = mRhs->evaluate();

        switch(mOp) {
            case ComparisonOperator::EQUAL:
                return lhs == rhs;
            case ComparisonOperator::NOT_EQUAL:
                return lhs != rhs;
            case ComparisonOperator::LESS_THAN:
                return lhs < rhs;
            case ComparisonOperator::LESS_OR_EQUAL:
                return lhs <= rhs;
            case ComparisonOperator::GREATER_THAN:
                return lhs > rhs;
            case ComparisonOperator::GREATER_OR_EQUAL:
                return lhs >= rhs;
            case ComparisonOperator::CONTAINS:
                if(lhs.type() == ValueType::STRING && rhs.type() == ValueType::STRING) {
                    return lhs.as_string().find(rhs.as_string()) != std::string::npos;
                }
                return false;
        }

        throw Exception::General("Unknown comparison operator");
    }

private:
    ComparisonOperator mOp;
    std::shared_ptr<CompiledValueExpression> mLhs;
    std::shared_ptr<CompiledValueExpression> mRhs;
};

class LogicalConditionEvaluator : public CompiledConditionEvaluator {
public:
    LogicalConditionEvaluator(LogicalOperator op, std::shared_ptr<ConditionalAssignment::ConditionEvaluator> lhs, std::shared_ptr<ConditionalAssignment::ConditionEvaluator> rhs)
        : mOp(op)
        , mLhs(std::move(lhs))
        , mRhs(std::move(rhs)) {}

    bool evaluate() override {
        switch(mOp) {
            case LogicalOperator::AND:
                return mLhs->evaluate() && mRhs->evaluate();
            case LogicalOperator::OR:
                return mLhs->evaluate() || mRhs->evaluate();
        }

        throw Exception::General("Unknown logical operator");
    }

private:
    LogicalOperator mOp;
    std::shared_ptr<ConditionalAssignment::ConditionEvaluator> mLhs;
    std::shared_ptr<ConditionalAssignment::ConditionEvaluator> mRhs;
};

class CompiledAssignmentEvaluator : public ConditionalAssignment::AssignmentEvaluator {
public:
    CompiledAssignmentEvaluator(std::string target, std::shared_ptr<CompiledValueExpression> expr)
        : mTarget(std::move(target))
        , mExpr(std::move(expr)) {}

    ResolvedProperty evaluate() override {
        return {mTarget, mExpr->evaluate().as_string()};
    }

    std::string name() const override {
        return mTarget;
    }

private:
    std::string mTarget;
    std::shared_ptr<CompiledValueExpression> mExpr;
};

std::shared_ptr<CompiledValueExpression> compile_value(const ValueExpressionPtr& expr, const std::map<std::string, std::shared_ptr<Token>>& token_map) {
    if(auto identifier = std::dynamic_pointer_cast<IdentifierExpression>(expr)) {
        auto it = token_map.find(identifier->name);
        if(it == token_map.end()) {
            throw Exception::General("Unknown identifier in dynamic property expression [" + identifier->name + "]");
        }

        return std::make_shared<TokenValueExpression>(it->second);
    }

    if(auto string_literal = std::dynamic_pointer_cast<StringLiteralExpression>(expr)) {
        return std::make_shared<LiteralValueExpression>(BaseValue(string_literal->value));
    }

    if(auto number_literal = std::dynamic_pointer_cast<NumberLiteralExpression>(expr)) {
        return std::make_shared<LiteralValueExpression>(BaseValue(number_literal->value));
    }

    if(auto binary = std::dynamic_pointer_cast<BinaryValueExpression>(expr)) {
        return std::make_shared<BinaryCompiledValueExpression>(binary->op, compile_value(binary->lhs, token_map), compile_value(binary->rhs, token_map));
    }

    throw Exception::General("Unknown dynamic property value expression");
}

std::shared_ptr<ConditionalAssignment::ConditionEvaluator> compile_condition(const BooleanExpressionPtr& expr, const std::map<std::string, std::shared_ptr<Token>>& token_map) {
    if(auto comparison = std::dynamic_pointer_cast<ComparisonExpression>(expr)) {
        return std::make_shared<ComparisonConditionEvaluator>(comparison->op, compile_value(comparison->lhs, token_map), compile_value(comparison->rhs, token_map));
    }

    if(auto logical = std::dynamic_pointer_cast<LogicalExpression>(expr)) {
        return std::make_shared<LogicalConditionEvaluator>(logical->op, compile_condition(logical->lhs, token_map), compile_condition(logical->rhs, token_map));
    }

    throw Exception::General("Unknown dynamic property boolean expression");
}

std::vector<std::shared_ptr<ConditionalAssignment::AssignmentEvaluator>> compile_assignments(const std::vector<AssignmentStatement>& assignments, const std::map<std::string, std::shared_ptr<Token>>& token_map) {
    std::vector<std::shared_ptr<ConditionalAssignment::AssignmentEvaluator>> compiled;
    compiled.reserve(assignments.size());

    for(const auto& assignment : assignments) {
        compiled.push_back(std::make_shared<CompiledAssignmentEvaluator>(assignment.target, compile_value(assignment.expr, token_map)));
    }

    return compiled;
}

} // namespace

Driver::Driver(std::string input)
    : mInput(std::move(input))
    , mError()
    , mResult(nullptr)
    , mScanner(nullptr) {
    yyscan_t scanner = nullptr;
    if(yylex_init(&scanner) != 0) {
        throw Exception::General("Failed to initialize dynamic property lexer");
    }

    mScanner = scanner;
}

Driver::~Driver() {
    if(mScanner != nullptr) {
        yylex_destroy(static_cast<yyscan_t>(mScanner));
    }
}

ConditionalAstPtr Driver::parse() {
    YY_BUFFER_STATE buffer = yy_scan_string(mInput.c_str(), static_cast<yyscan_t>(mScanner));
    Parser parser(*this, static_cast<yyscan_t>(mScanner));
    int ret = parser.parse();
    yy_delete_buffer(buffer, static_cast<yyscan_t>(mScanner));

    if(ret != 0) {
        if(mError.empty()) {
            throw Exception::General("Failed to parse dynamic property expression");
        }

        throw Exception::General(mError);
    }

    if(!mResult) {
        throw Exception::General("Dynamic property parser produced no result");
    }

    return mResult;
}

std::shared_ptr<ConditionalAssignment> compile(const std::string& source, const std::map<std::string, std::shared_ptr<Token>>& token_map) {
    Driver driver(source);
    ConditionalAstPtr ast = driver.parse();

    return std::make_shared<ConditionalAssignment>(
        compile_condition(ast->condition, token_map),
        compile_assignments(ast->then_assignments, token_map),
        compile_assignments(ast->else_assignments, token_map)
    );
}

} // namespace DynamicProperty
