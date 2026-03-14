#ifndef CONDITIONAL_ASSIGNMENT_H
#define CONDITIONAL_ASSIGNMENT_H
/**
 * @file ConditionalAssignment.h
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief Represents a simple if then else structure for dynamic assignment of properties
 * If the BoolExpression evaluates to true, mThen will be evaluated. If there is an mElse, that
 * expression will be evaluated if the "if" BoolExpression evalutes to false.
 * @version 1.0
 * @date 30-04-2022
 *
 * @copyright Copyright (C) Barry Robinson 2022
 *
 */

#include <memory>
#include <vector>
#include "Assignment.h"
#include "Variable.h"
#include "BoolExpression.h"
#include "ResolvedProperty.h"

class ConditionalAssignment {
public:
    class ConditionEvaluator {
    public:
        virtual ~ConditionEvaluator() = default;
        virtual bool evaluate() = 0;
    };

    class AssignmentEvaluator {
    public:
        virtual ~AssignmentEvaluator() = default;
        virtual ResolvedProperty evaluate() = 0;
        virtual std::string name() const = 0;
    };

    ConditionalAssignment(std::shared_ptr<BoolExpression> ifEx, std::vector<std::shared_ptr<Assignment>> thenEx, std::vector<std::shared_ptr<Assignment>> elseEx = {})
        : mExp(std::make_shared<LegacyConditionEvaluator>(std::move(ifEx)))
        , mThen(adapt_assignments(thenEx))
        , mElse(adapt_assignments(elseEx))
        , mResults()
        , mRet(false) {}

    ConditionalAssignment(std::shared_ptr<ConditionEvaluator> ifEx, std::vector<std::shared_ptr<AssignmentEvaluator>> thenEx, std::vector<std::shared_ptr<AssignmentEvaluator>> elseEx = {})
        : mExp(std::move(ifEx))
        , mThen(std::move(thenEx))
        , mElse(std::move(elseEx))
        , mResults()
        , mRet(false) {}


    /**
     * @brief Evaluate a list of expressions which all constitute an OR clause
     * if any of them match then execut the THEN clause. If they don't, and there is
     * an ELSE clause, execute that.
     *
     */
    bool evaluate() {
        mResults.clear();

        mRet = mExp->evaluate();

        if(mRet) {
            for(auto& a : mThen) {
                mResults.push_back(a->evaluate());
            }
        }
        else {
            for(auto& a : mElse) {
                mResults.push_back(a->evaluate());
            }
        }

        return mRet;
    }

    /**
     * @brief Get the values object as a reference so that when it is updated
     * the list will be remade
     *
     * @return std::vector<ResolvedProperty>&
     */

    std::vector<ResolvedProperty>& get_results() {
        return mResults;
    }


    std::vector<ResolvedProperty> get_expression_properies() {
        std::vector<ResolvedProperty> props {};

        for(auto as : mThen) {
            props.push_back({as->name(), ""});
        }

        for(auto as : mElse) {
            props.push_back({as->name(), ""});
        }

        return props;
    }

private:
    class LegacyConditionEvaluator : public ConditionEvaluator {
    public:
        explicit LegacyConditionEvaluator(std::shared_ptr<BoolExpression> expression)
            : mExpression(std::move(expression)) {}

        bool evaluate() override {
            return mExpression->evaluate();
        }

    private:
        std::shared_ptr<BoolExpression> mExpression;
    };

    class LegacyAssignmentEvaluator : public AssignmentEvaluator {
    public:
        explicit LegacyAssignmentEvaluator(std::shared_ptr<Assignment> assignment)
            : mAssignment(std::move(assignment)) {}

        ResolvedProperty evaluate() override {
            std::shared_ptr<Variable> value = mAssignment->evaluate();
            return {value->name(), value->as_string()};
        }

        std::string name() const override {
            return mAssignment->get_assignmet_variable()->name();
        }

    private:
        std::shared_ptr<Assignment> mAssignment;
    };

    static std::vector<std::shared_ptr<AssignmentEvaluator>> adapt_assignments(const std::vector<std::shared_ptr<Assignment>>& assignments) {
        std::vector<std::shared_ptr<AssignmentEvaluator>> ret;
        ret.reserve(assignments.size());
        for(const auto& assignment : assignments) {
            ret.push_back(std::make_shared<LegacyAssignmentEvaluator>(assignment));
        }

        return ret;
    }

    std::shared_ptr<ConditionEvaluator> mExp;
    std::vector<std::shared_ptr<AssignmentEvaluator>> mThen;     // Could be multiple assignments
    std::vector<std::shared_ptr<AssignmentEvaluator>> mElse;
    std::vector<ResolvedProperty> mResults;
    bool mRet;
};

#endif//CONDITIONAL_ASSIGNMENT_H
