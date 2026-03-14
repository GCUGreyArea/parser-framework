#ifndef PARSER_FRAMEWORK_PARSER_FRAMEWORK_HPP
#define PARSER_FRAMEWORK_PARSER_FRAMEWORK_HPP

#include <cstddef>
#include <map>
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
    std::string event_name;
    std::string event_pattern_id;
    std::vector<Token> tokens;
    std::vector<std::string> errors;
};

struct RegexTokenRule {
    std::string name;
    std::string pattern;
    std::size_t group {1};
};

struct DeclaredToken {
    std::string name;
    std::string type;
};

struct NamedCapture {
    std::string name;
    std::size_t group {0};
};

struct MessageFilterRule {
    std::string pattern;
    std::vector<NamedCapture> captures;
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
    std::string extract_name;
    std::string ruleset_name;
    std::optional<std::string> locator_pattern;
    std::size_t locator_group {1};
    bool allow_multiple {false};
    std::vector<DeclaredToken> declared_tokens;
    std::vector<RegexTokenRule> regex_tokens;
    std::vector<JsonMatchRule> json_matches;
    std::vector<JsonTokenRule> json_tokens;
    std::vector<SectionRule> children;
};

struct MessageRule {
    std::string id;
    std::string name;
    std::vector<MessageFilterRule> filters;
    std::vector<SectionRule> sections;
};

class ParserFramework {
public:
    ParserFramework() = default;
    explicit ParserFramework(std::vector<MessageRule> message_rules);

    void add_message_rule(MessageRule message_rule);
    ParseResult parse_message(const std::string& message) const;
    std::vector<ParseResult> parse_messages(const std::vector<std::string>& messages) const;

private:
    std::vector<MessageRule> m_message_rules;
};

std::string render_results_as_json(const std::vector<ParseResult>& results);
std::string render_result_as_json(const ParseResult& result);

} // namespace parser_framework

#endif
