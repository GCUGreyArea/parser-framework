#include "parser_framework/ParsingEngines.hpp"

#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include "jqpath.h"
#include "jsmn.hpp"

namespace parser_framework {

namespace {

std::string json_token_to_string(const std::string& json, const jsmntok_t* token) {
    if (token == nullptr || token->start < 0 || token->end < token->start) {
        return {};
    }

    std::string value = json.substr(static_cast<std::size_t>(token->start),
        static_cast<std::size_t>(token->end - token->start));
    if (token->type == JSMN_STRING && value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }

    return value;
}

std::optional<std::string> lookup_json_value(jsmn_parser& parser,
    const std::string& json,
    const std::string& path_expression) {
    struct jqpath* path = jsonpath_parse_string(path_expression.c_str());
    if (path == nullptr) {
        throw std::runtime_error("Failed to parse JSONPath [" + path_expression + "]");
    }

    JQ* value = parser.get_path(path);
    const jsmntok_t* token = value == nullptr ? nullptr : value->get_token();
    std::optional<std::string> rendered;
    if (token != nullptr) {
        rendered = json_token_to_string(json, token);
    }

    jqpath_close_path(path);
    return rendered;
}

bool json_match_succeeds(const SectionRule& section,
    jsmn_parser& parser,
    const std::string& json,
    std::vector<std::string>& errors) {
    for (const auto& match_rule : section.json_matches) {
        try {
            const std::optional<std::string> value = lookup_json_value(parser, json, match_rule.path);
            if (!value.has_value()) {
                return false;
            }

            if (match_rule.equals.has_value() && *value != *match_rule.equals) {
                return false;
            }
        } catch (const std::exception& ex) {
            errors.push_back(ex.what());
            return false;
        }
    }

    return true;
}

} // namespace

bool JSONParsingEngine::parse_section(
    const SectionRule& section,
    const SectionSlice& slice,
    ParseResult& result) const {
    jsmn_parser parser(slice.text.c_str(), 2);
    const int parse_ret = parser.parse();
    if (parse_ret < 0) {
        std::ostringstream stream;
        stream << "JSON parse failed in section [" << section.id << "] with error " << parse_ret;
        result.errors.push_back(stream.str());
        return false;
    }

    if (!json_match_succeeds(section, parser, slice.text, result.errors)) {
        return false;
    }

    bool matched = false;
    for (const auto& token_rule : section.json_tokens) {
        try {
            struct jqpath* path = jsonpath_parse_string(token_rule.path.c_str());
            if (path == nullptr) {
                result.errors.push_back("Failed to parse JSONPath [" + token_rule.path + "]");
                continue;
            }

            JQ* value = parser.get_path(path);
            const jsmntok_t* token = value == nullptr ? nullptr : value->get_token();
            if (token != nullptr) {
                result.tokens.push_back(Token{
                    token_rule.name,
                    json_token_to_string(slice.text, token),
                    section.id,
                    "jsonpath",
                    token_rule.path,
                    slice.start + static_cast<std::size_t>(token->start),
                    slice.start + static_cast<std::size_t>(token->end)
                });
                matched = true;
            }

            jqpath_close_path(path);
        } catch (const std::exception& ex) {
            result.errors.push_back(ex.what());
        }
    }

    return matched;
}

std::unordered_map<std::string, std::string> JSONParsingEngine::parse_values(
    const SectionRule& section,
    const std::string& message,
    std::vector<std::string>& errors) const {
    std::unordered_map<std::string, std::string> values;

    jsmn_parser parser(message.c_str(), 2);
    if (parser.parse() < 0) {
        errors.push_back("JSON example failed to parse");
        return values;
    }

    if (!json_match_succeeds(section, parser, message, errors)) {
        return values;
    }

    for (const auto& token_rule : section.json_tokens) {
        try {
            const std::optional<std::string> value = lookup_json_value(parser, message, token_rule.path);
            if (value.has_value()) {
                values[token_rule.name] = *value;
            }
        } catch (const std::exception& ex) {
            errors.push_back(ex.what());
        }
    }

    return values;
}

} // namespace parser_framework
