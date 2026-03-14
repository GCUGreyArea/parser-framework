#ifndef PARSER_FRAMEWORK_PARSER_FRAMEWORK_HPP
#define PARSER_FRAMEWORK_PARSER_FRAMEWORK_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace parser_framework {

enum class SectionKind {
    FREE_TEXT,
    KV,
    JSON,
    MIXED
};

struct Token {
    std::string name;
    std::string value;
    std::string section;
    std::string source_engine;
    std::string path;
    std::size_t start {0};
    std::size_t end {0};
};

struct ParseResult {
    bool matched {false};
    std::string message_rule_id;
    std::string message_rule_name;
    std::vector<Token> tokens;
    std::vector<std::string> errors;
};

struct RegexTokenRule {
    std::string name;
    std::string pattern;
    std::size_t group {1};
};

struct JsonMatchRule {
    std::string path;
    std::optional<std::string> equals;
};

struct JsonTokenRule {
    std::string name;
    std::string path;
};

struct SectionRule {
    std::string id;
    SectionKind kind {SectionKind::FREE_TEXT};
    std::optional<std::string> locator_pattern;
    std::size_t locator_group {1};
    bool allow_multiple {false};
    std::vector<RegexTokenRule> regex_tokens;
    std::vector<JsonMatchRule> json_matches;
    std::vector<JsonTokenRule> json_tokens;
    std::vector<SectionRule> children;
};

struct MessageRule {
    std::string id;
    std::string name;
    std::vector<std::string> anchor_patterns;
    std::vector<SectionRule> sections;
};

class ParserFramework {
public:
    ParserFramework() = default;
    explicit ParserFramework(std::vector<MessageRule> message_rules);

    void add_message_rule(MessageRule message_rule);
    ParseResult parse_message(const std::string& message) const;

private:
    std::vector<MessageRule> m_message_rules;
};

} // namespace parser_framework

#endif
