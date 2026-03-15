#include "parser_framework/DynamicPropertyEngine.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "DynamicProperty/Compiler.h"
#include "Framework/Match.h"
#include "Framework/Token.h"
#include "Framework/ValueType.h"

namespace parser_framework {

namespace {

struct BoundToken {
    std::shared_ptr<::Token> token;
    std::shared_ptr<Match> match;
};

bool is_integer_string(const std::string& value) {
    return !value.empty() &&
        std::all_of(value.begin(), value.end(), [](unsigned char ch) {
            return ch >= '0' && ch <= '9';
        });
}

BoundToken bind_token(const std::string& name, const std::string& value) {
    const bool is_int = is_integer_string(value);
    const std::string pattern = is_int ? "^([0-9]+)$" : "^(.*)$";
    const ValueType type = is_int ? ValueType::INT : ValueType::STRING;

    BoundToken bound;
    bound.token = std::make_shared<::Token>(name, pattern, type, 0);
    bound.match = std::make_shared<Match>(value);
    bound.match->set_match_type(Match::Type::SEQUENTIAL);
    bound.token->match(*bound.match);
    return bound;
}

std::map<std::string, std::shared_ptr<::Token>> build_token_map(
    const ParseResult& result,
    std::vector<std::shared_ptr<Match>>& match_storage) {
    std::map<std::string, std::shared_ptr<::Token>> token_map;

    for (const auto& token : result.tokens) {
        BoundToken bound = bind_token(token.name, token.value);
        match_storage.push_back(bound.match);
        token_map[token.name] = std::move(bound.token);
    }

    for (const auto& property : result.properties) {
        BoundToken bound = bind_token(property.first, property.second);
        match_storage.push_back(bound.match);
        token_map[property.first] = std::move(bound.token);
    }

    for (const auto& metadata : {
             std::pair<std::string, std::string>{"message_rule_id", result.message_rule_id},
             {"message_rule_name", result.message_rule_name},
             {"event_name", result.event_name},
             {"event_pattern_id", result.event_pattern_id}}) {
        BoundToken bound = bind_token(metadata.first, metadata.second);
        match_storage.push_back(bound.match);
        token_map[metadata.first] = std::move(bound.token);
    }

    return token_map;
}

} // namespace

std::map<std::string, std::string> DynamicPropertyEngine::evaluate(
    const std::vector<std::string>& expressions,
    const ParseResult& result,
    std::vector<std::string>& errors) const {
    std::map<std::string, std::string> properties;
    std::vector<std::shared_ptr<Match>> match_storage;
    const std::map<std::string, std::shared_ptr<::Token>> token_map =
        build_token_map(result, match_storage);

    for (const auto& expression : expressions) {
        try {
            std::shared_ptr<ConditionalAssignment> compiled = DynamicProperty::compile(expression, token_map);
            compiled->evaluate();

            for (const auto& property : compiled->get_results()) {
                properties[property.name] = property.value;
            }
        } catch (const std::exception& ex) {
            errors.push_back("Dynamic property evaluation failed [" + expression + "]: " + ex.what());
        }
    }

    return properties;
}

} // namespace parser_framework
