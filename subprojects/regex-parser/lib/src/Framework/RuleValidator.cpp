#include "RuleValidator.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Pattern.h"
#include "Rule.h"
#include "Token.h"
#include "Exception/Exceptions.h"

namespace RuleValidation {

namespace {

struct RuleMatchResult {
    std::string rule_id;
    std::string pattern_id;
};

std::map<std::string, std::string> to_property_map(const std::vector<ResolvedProperty>& properties)
{
    std::map<std::string, std::string> values;
    for (const auto& property : properties)
    {
        values[property.name] = property.value;
    }

    return values;
}

std::string example_prefix(Rule& rule, Pattern& pattern, size_t example_index)
{
    return "Rule [" + rule.id() + "] pattern [" + pattern.id() + "] example [" + std::to_string(example_index) + "]";
}

std::vector<RuleMatchResult> collect_matches(std::vector<std::unique_ptr<Rule>>& rules, const std::string& message)
{
    std::vector<RuleMatchResult> matches;

    for (auto& rule : rules)
    {
        Match match(message);
        Pattern* matched_pattern = rule->match(match);
        if (matched_pattern != nullptr)
        {
            matches.push_back({rule->id(), matched_pattern->id()});
        }
    }

    return matches;
}

void validate_expected_tokens(const std::string& prefix, Pattern& pattern, const PatternExample& example)
{
    std::map<std::string, std::shared_ptr<Token>> token_map;
    std::vector<std::shared_ptr<Token>>& tokens = pattern.get_tokens();
    for (auto& token : tokens)
    {
        if (!token->matched())
        {
            throw Exception::ExampleMessageDoesntMatch(
                prefix + " did not extract token [" + token->name() + "]"
            );
        }

        token_map.emplace(token->name(), token);
    }

    for (const auto& expected_token : example.expected_tokens)
    {
        auto token_it = token_map.find(expected_token.first);
        if (token_it == token_map.end())
        {
            throw Exception::ExampleMessageDoesntMatch(
                prefix + " expected undeclared token [" + expected_token.first + "]"
            );
        }

        if (token_it->second->as_string() != expected_token.second)
        {
            throw Exception::ExampleMessageDoesntMatch(
                prefix + " token [" + expected_token.first + "] expected [" + expected_token.second +
                "] but parsed [" + token_it->second->as_string() + "]"
            );
        }
    }
}

void validate_expected_properties(const std::string& prefix, Pattern& pattern, const PatternExample& example)
{
    std::map<std::string, std::string> properties = to_property_map(pattern.get_properties());

    for (const auto& expected_property : example.expected_properties)
    {
        auto property_it = properties.find(expected_property.first);
        if (property_it == properties.end())
        {
            throw Exception::ExampleMessageDoesntMatch(
                prefix + " did not emit property [" + expected_property.first + "]"
            );
        }

        if (property_it->second != expected_property.second)
        {
            throw Exception::ExampleMessageDoesntMatch(
                prefix + " property [" + expected_property.first + "] expected [" + expected_property.second +
                "] but parsed [" + property_it->second + "]"
            );
        }
    }

    for (const auto& absent_property : example.absent_properties)
    {
        if (properties.find(absent_property) != properties.end())
        {
            throw Exception::ExampleMessageDoesntMatch(
                prefix + " unexpectedly emitted property [" + absent_property + "]"
            );
        }
    }
}

} // namespace

void validate_examples(std::vector<std::unique_ptr<Rule>>& rules)
{
    for (auto& expected_rule : rules)
    {
        for (auto& expected_pattern : expected_rule->patterns())
        {
            const std::vector<PatternExample>& examples = expected_pattern.examples();
            for (size_t example_index = 0; example_index < examples.size(); ++example_index)
            {
                const PatternExample& example = examples.at(example_index);
                const std::string prefix = example_prefix(*expected_rule, expected_pattern, example_index);
                std::vector<RuleMatchResult> matches = collect_matches(rules, example.message);

                if (matches.empty())
                {
                    throw Exception::ExampleMessageDoesntMatch(prefix + " did not match any rule");
                }

                if (matches.size() != 1)
                {
                    std::string conflict = prefix + " matched multiple rules:";
                    for (const auto& match : matches)
                    {
                        conflict += " [" + match.rule_id + "/" + match.pattern_id + "]";
                    }

                    throw Exception::ExampleMessageDoesntMatch(conflict);
                }

                if (matches.front().rule_id != expected_rule->id())
                {
                    throw Exception::ExampleMessageDoesntMatch(
                        prefix + " matched rule [" + matches.front().rule_id + "] instead of [" + expected_rule->id() + "]"
                    );
                }

                if (matches.front().pattern_id != expected_pattern.id())
                {
                    throw Exception::ExampleMessageDoesntMatch(
                        prefix + " matched pattern [" + matches.front().pattern_id + "] instead of [" + expected_pattern.id() + "]"
                    );
                }

                Match expected_match(example.message);
                Pattern* matched_pattern = expected_rule->match(expected_match);
                if (matched_pattern == nullptr)
                {
                    throw Exception::ExampleMessageDoesntMatch(
                        prefix + " could not be re-matched for token and property validation"
                    );
                }

                validate_expected_tokens(prefix, *matched_pattern, example);
                validate_expected_properties(prefix, *matched_pattern, example);
            }
        }
    }
}

} // namespace RuleValidation
