#include "parser_framework/RuleLoader.hpp"

#include <algorithm>
#include <filesystem>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "parser_framework/DynamicPropertyEngine.hpp"
#include "parser_framework/MessageLoader.hpp"
#include "parser_framework/ParsingEngines.hpp"

namespace parser_framework {
namespace RuleLoader {

namespace {

enum class RulesetParserKind {
    KV,
    JSON,
    REGEX
};

struct ParserRuleSet {
    std::string id;
    std::string name;
    RulesetParserKind kind {RulesetParserKind::KV};
    std::filesystem::path definition_dir;
    std::unordered_set<std::string> bindings;
    std::vector<DeclaredToken> tokens;
    std::vector<std::string> dynamic_properties;
    std::vector<RegexTokenRule> regex_tokens;
    std::vector<JsonMatchRule> json_matches;
    std::vector<JsonTokenRule> json_tokens;
    YAML::Node examples;
};

struct IdentificationExample {
    std::string message;
    std::unordered_map<std::string, std::string> captures;
};

std::filesystem::path resolve_message_path(const std::filesystem::path& base_dir, const std::string& reference) {
    namespace fs = std::filesystem;

    const fs::path input(reference);
    if (input.is_absolute()) {
        return input;
    }

    for (fs::path current = base_dir; !current.empty(); ) {
        const fs::path candidate = current / input;
        if (fs::exists(candidate)) {
            return candidate;
        }

        const fs::path parent = current.parent_path();
        if (parent == current) {
            break;
        }
        current = parent;
    }

    return input;
}

std::string load_example_message(const YAML::Node& node, const std::filesystem::path& base_dir) {
    if (node["message"]) {
        return node["message"].as<std::string>();
    }

    if (node["message_file"]) {
        return MessageLoader::load_message_file(resolve_message_path(base_dir, node["message_file"].as<std::string>()).string());
    }

    throw std::runtime_error("Example is missing message or message_file");
}

std::vector<std::string> parse_dynamic_properties(const YAML::Node& node) {
    std::vector<std::string> expressions;
    if (!node || !node.IsSequence()) {
        return expressions;
    }

    for (const auto& expression : node) {
        expressions.push_back(expression.as<std::string>());
    }

    return expressions;
}

RulesetParserKind parse_ruleset_kind(const std::string& parser_name) {
    if (parser_name == "kv") {
        return RulesetParserKind::KV;
    }
    if (parser_name == "json") {
        return RulesetParserKind::JSON;
    }
    if (parser_name == "regex") {
        return RulesetParserKind::REGEX;
    }

    throw std::runtime_error("Unsupported parser ruleset type [" + parser_name + "]");
}

MessageFilterRule normalize_message_filter(const std::string& pattern) {
    MessageFilterRule filter;
    std::size_t group = 0;
    bool escaped = false;

    for (std::size_t i = 0; i < pattern.size(); ++i) {
        const char ch = pattern[i];
        if (escaped) {
            filter.pattern.push_back(ch);
            escaped = false;
            continue;
        }

        if (ch == '\\') {
            filter.pattern.push_back(ch);
            escaped = true;
            continue;
        }

        if (ch == '(') {
            if (i + 2 < pattern.size() && pattern[i + 1] == '?' && pattern[i + 2] == '<') {
                const std::size_t name_start = i + 3;
                const std::size_t name_end = pattern.find('>', name_start);
                if (name_end == std::string::npos) {
                    throw std::runtime_error("Invalid named capture group in pattern [" + pattern + "]");
                }

                ++group;
                filter.captures.push_back(NamedCapture{pattern.substr(name_start, name_end - name_start), group});
                filter.pattern.push_back('(');
                i = name_end;
                continue;
            }

            if (!(i + 1 < pattern.size() && pattern[i + 1] == '?')) {
                ++group;
            }
        }

        filter.pattern.push_back(ch);
    }

    return filter;
}

std::vector<DeclaredToken> parse_declared_tokens(const YAML::Node& node) {
    std::vector<DeclaredToken> tokens;
    if (!node || !node.IsMap()) {
        return tokens;
    }

    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
        tokens.push_back(DeclaredToken{
            it->first.as<std::string>(),
            it->second.as<std::string>()
        });
    }

    return tokens;
}

std::vector<RegexTokenRule> parse_regex_token_rules(const YAML::Node& node) {
    std::vector<RegexTokenRule> rules;
    if (!node || !node.IsSequence()) {
        return rules;
    }

    for (const auto& token_node : node) {
        rules.push_back(RegexTokenRule{
            token_node["name"].as<std::string>(),
            token_node["pattern"].as<std::string>(),
            token_node["group"] ? token_node["group"].as<std::size_t>() : 1u
        });
    }

    return rules;
}

std::vector<JsonMatchRule> parse_json_match_rules(const YAML::Node& node) {
    std::vector<JsonMatchRule> rules;
    if (!node || !node.IsSequence()) {
        return rules;
    }

    for (const auto& match_node : node) {
        JsonMatchRule rule;
        rule.path = match_node["path"].as<std::string>();
        if (match_node["equals"]) {
            rule.equals = match_node["equals"].as<std::string>();
        }
        rules.push_back(std::move(rule));
    }

    return rules;
}

std::vector<JsonTokenRule> parse_json_token_rules(const YAML::Node& node) {
    std::vector<JsonTokenRule> rules;
    if (!node || !node.IsMap()) {
        return rules;
    }

    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
        rules.push_back(JsonTokenRule{
            it->first.as<std::string>(),
            it->second.as<std::string>()
        });
    }

    return rules;
}

ParserRuleSet parse_ruleset(const YAML::Node& node, const std::filesystem::path& definition_dir) {
    ParserRuleSet ruleset;
    ruleset.id = node["id"].as<std::string>();
    ruleset.name = node["name"].as<std::string>();
    ruleset.kind = parse_ruleset_kind(node["parser"].as<std::string>());
    ruleset.definition_dir = definition_dir;
    ruleset.tokens = parse_declared_tokens(node["tokens"]);
    ruleset.dynamic_properties = parse_dynamic_properties(node["dynamic_properties"]);
    ruleset.examples = node["examples"];

    if (node["bindings"]) {
        for (const auto& binding : node["bindings"]) {
            ruleset.bindings.insert(binding.as<std::string>());
        }
    }

    switch (ruleset.kind) {
    case RulesetParserKind::KV:
        break;
    case RulesetParserKind::JSON:
        ruleset.json_matches = parse_json_match_rules(node["json_matches"]);
        ruleset.json_tokens = parse_json_token_rules(node["json_tokens"]);
        break;
    case RulesetParserKind::REGEX:
        ruleset.regex_tokens = parse_regex_token_rules(node["regex_tokens"]);
        break;
    }

    return ruleset;
}

void validate_expected_tokens(const ParserRuleSet& ruleset,
    const YAML::Node& expected_tokens,
    const std::unordered_map<std::string, std::string>& parsed_tokens) {
    if (!expected_tokens || !expected_tokens.IsMap()) {
        return;
    }

    for (YAML::const_iterator it = expected_tokens.begin(); it != expected_tokens.end(); ++it) {
        const std::string name = it->first.as<std::string>();
        const std::string expected = it->second.as<std::string>();
        auto parsed_it = parsed_tokens.find(name);
        if (parsed_it == parsed_tokens.end()) {
            throw std::runtime_error(
                "Example for ruleset [" + ruleset.name + "] missing token [" + name + "]");
        }
        if (parsed_it->second != expected) {
            throw std::runtime_error(
                "Example for ruleset [" + ruleset.name + "] token [" + name + "] expected [" +
                expected + "] but parsed [" + parsed_it->second + "]");
        }
    }
}

void validate_absent_tokens(const ParserRuleSet& ruleset,
    const YAML::Node& absent_tokens,
    const std::unordered_map<std::string, std::string>& parsed_tokens) {
    if (!absent_tokens || !absent_tokens.IsSequence()) {
        return;
    }

    for (const auto& absent : absent_tokens) {
        const std::string name = absent.as<std::string>();
        if (parsed_tokens.find(name) != parsed_tokens.end()) {
            throw std::runtime_error(
                "Example for ruleset [" + ruleset.name + "] unexpectedly parsed absent token [" + name + "]");
        }
    }
}

void validate_ruleset_examples(const ParserRuleSet& ruleset) {
    if (!ruleset.examples || !ruleset.examples.IsSequence()) {
        return;
    }

    const RegexParsingEngine regex_engine;
    const KVParsingEngine kv_engine;
    const JSONParsingEngine json_engine;

    const DynamicPropertyEngine property_engine;

    for (const auto& example : ruleset.examples) {
        const std::string message = load_example_message(example, ruleset.definition_dir);
        std::unordered_map<std::string, std::string> parsed_tokens;
        std::vector<std::string> errors;

        switch (ruleset.kind) {
        case RulesetParserKind::KV:
            parsed_tokens = kv_engine.parse_values(ruleset.tokens, message);
            break;
        case RulesetParserKind::JSON:
        {
            SectionRule section;
            section.id = ruleset.id;
            section.kind = SectionKind::JSON;
            section.json_matches = ruleset.json_matches;
            section.json_tokens = ruleset.json_tokens;
            parsed_tokens = json_engine.parse_values(section, message, errors);
            break;
        }
        case RulesetParserKind::REGEX:
            parsed_tokens = regex_engine.parse_values(ruleset.regex_tokens, message, errors);
            break;
        }

        if (!errors.empty()) {
            throw std::runtime_error(
                "Example for ruleset [" + ruleset.name + "] failed: " + errors.front());
        }

        validate_expected_tokens(ruleset, example["expect"]["tokens"], parsed_tokens);
        validate_absent_tokens(ruleset, example["expect"]["absent_tokens"], parsed_tokens);

        if (!ruleset.dynamic_properties.empty()) {
            ParseResult result;
            for (const auto& parsed : parsed_tokens) {
                result.tokens.push_back(Token{parsed.first, parsed.second, ruleset.name, "example", parsed.first, 0, 0});
            }

            const std::map<std::string, std::string> properties =
                property_engine.evaluate(ruleset.dynamic_properties, result, errors);
            if (!errors.empty()) {
                throw std::runtime_error(
                    "Dynamic property example for ruleset [" + ruleset.name + "] failed: " + errors.front());
            }

            const YAML::Node expected_properties = example["expect"]["properties"];
            if (expected_properties && expected_properties.IsMap()) {
                for (YAML::const_iterator it = expected_properties.begin(); it != expected_properties.end(); ++it) {
                    const std::string name = it->first.as<std::string>();
                    const std::string expected = it->second.as<std::string>();
                    auto property_it = properties.find(name);
                    if (property_it == properties.end()) {
                        throw std::runtime_error(
                            "Example for ruleset [" + ruleset.name + "] missing property [" + name + "]");
                    }
                    if (property_it->second != expected) {
                        throw std::runtime_error(
                            "Example for ruleset [" + ruleset.name + "] property [" + name + "] expected [" +
                            expected + "] but parsed [" + property_it->second + "]");
                    }
                }
            }
        }
    }
}

IdentificationExample parse_identification_example(const YAML::Node& node, const std::filesystem::path& definition_dir) {
    IdentificationExample example;
    example.message = load_example_message(node, definition_dir);

    const YAML::Node captures = node["expect"]["captures"];
    if (captures && captures.IsMap()) {
        for (YAML::const_iterator it = captures.begin(); it != captures.end(); ++it) {
            example.captures.emplace(it->first.as<std::string>(), it->second.as<std::string>());
        }
    }

    return example;
}

void validate_identification_examples(const MessageRule& rule, const std::vector<IdentificationExample>& examples) {
    const RegexParsingEngine regex_engine;

    for (const auto& example : examples) {
        FilterMatch match;
        std::vector<std::string> errors;

        for (const auto& filter : rule.filters) {
            match = regex_engine.match_message_filter(filter, example.message, errors);
            if (match.matched) {
                break;
            }
        }

        if (!errors.empty()) {
            throw std::runtime_error(
                "Identification example for rule [" + rule.name + "] failed: " + errors.front());
        }

        if (!match.matched) {
            throw std::runtime_error("Identification example for rule [" + rule.name + "] did not match");
        }

        for (const auto& expected : example.captures) {
            auto capture_it = match.captures.find(expected.first);
            if (capture_it == match.captures.end()) {
                throw std::runtime_error(
                    "Identification example for rule [" + rule.name + "] missing capture [" + expected.first + "]");
            }

            if (capture_it->second != expected.second) {
                throw std::runtime_error(
                    "Identification example for rule [" + rule.name + "] capture [" + expected.first +
                    "] expected [" + expected.second + "] but parsed [" + capture_it->second + "]");
            }
        }
    }
}

void apply_locator_settings(SectionRule& section, const YAML::Node& node) {
    if (node["locator_pattern"]) {
        section.locator_pattern = node["locator_pattern"].as<std::string>();
    }
    if (node["locator_group"]) {
        section.locator_group = node["locator_group"].as<std::size_t>();
    }
    if (node["allow_multiple"]) {
        section.allow_multiple = node["allow_multiple"].as<bool>();
    }

    const YAML::Node locator = node["locator"];
    if (!locator || !locator.IsMap()) {
        return;
    }

    if (locator["pattern"]) {
        section.locator_pattern = locator["pattern"].as<std::string>();
    }
    if (locator["group"]) {
        section.locator_group = locator["group"].as<std::size_t>();
    }
    if (locator["multiple"]) {
        section.allow_multiple = locator["multiple"].as<bool>();
    }
}

SectionRule parse_section_node(const YAML::Node& node,
    const MessageRule& message_rule,
    const std::unordered_map<std::string, ParserRuleSet>& rulesets) {
    SectionRule section;
    if (node["id"]) {
        section.id = node["id"].as<std::string>();
    } else if (node["extract"]) {
        section.id = node["extract"].as<std::string>();
    } else if (node["ruleset"]) {
        section.id = node["ruleset"].as<std::string>();
    } else {
        throw std::runtime_error("Section is missing an id, extract, or ruleset reference");
    }

    if (node["extract"]) {
        section.extract_name = node["extract"].as<std::string>();
    }
    if (node["ruleset"]) {
        section.ruleset_name = node["ruleset"].as<std::string>();
    }
    apply_locator_settings(section, node);

    const std::string parser = node["parser"].as<std::string>();
    if (parser == "mixed") {
        section.kind = SectionKind::MIXED;
        const YAML::Node children = node["sections"];
        if (!children || !children.IsSequence() || children.size() == 0) {
            throw std::runtime_error("Mixed section [" + section.id + "] has no child sections");
        }

        for (const auto& child : children) {
            section.children.push_back(parse_section_node(child, message_rule, rulesets));
        }

        return section;
    }

    if (section.ruleset_name.empty()) {
        throw std::runtime_error("Section [" + section.id + "] is missing a ruleset reference");
    }

    auto ruleset_it = rulesets.find(section.ruleset_name);
    if (ruleset_it == rulesets.end()) {
        throw std::runtime_error("Unknown ruleset [" + section.ruleset_name + "]");
    }

    const ParserRuleSet& ruleset = ruleset_it->second;
    section.ruleset_id = ruleset.id;
    if (!ruleset.bindings.empty() && ruleset.bindings.find(message_rule.id) == ruleset.bindings.end()) {
        throw std::runtime_error(
            "Ruleset [" + section.ruleset_name + "] is not bound to identification rule [" + message_rule.id + "]");
    }

    if (parser == "kv" && ruleset.kind == RulesetParserKind::KV) {
        section.kind = SectionKind::KV;
        section.declared_tokens = ruleset.tokens;
        section.dynamic_properties = ruleset.dynamic_properties;
        return section;
    }

    if (parser == "json" && ruleset.kind == RulesetParserKind::JSON) {
        section.kind = SectionKind::JSON;
        section.declared_tokens = ruleset.tokens;
        section.dynamic_properties = ruleset.dynamic_properties;
        section.json_matches = ruleset.json_matches;
        section.json_tokens = ruleset.json_tokens;
        return section;
    }

    if (parser == "regex" && ruleset.kind == RulesetParserKind::REGEX) {
        section.kind = SectionKind::FREE_TEXT;
        section.declared_tokens = ruleset.tokens;
        section.dynamic_properties = ruleset.dynamic_properties;
        section.regex_tokens = ruleset.regex_tokens;
        return section;
    }

    throw std::runtime_error(
        "Section [" + section.id + "] parser [" + parser + "] does not match ruleset [" + section.ruleset_name + "]");
}

MessageRule parse_message_rule(const YAML::Node& node,
    const std::unordered_map<std::string, ParserRuleSet>& rulesets,
    const std::filesystem::path& definition_dir) {
    MessageRule rule;
    rule.id = node["id"].as<std::string>();
    rule.name = node["name"].as<std::string>();

    const YAML::Node filters = node["message_filter"];
    if (!filters || !filters.IsSequence() || filters.size() == 0) {
        throw std::runtime_error("Message rule [" + rule.id + "] has no message_filter patterns");
    }

    for (const auto& filter_node : filters) {
        rule.filters.push_back(normalize_message_filter(filter_node["pattern"].as<std::string>()));
    }

    const YAML::Node sections = node["sections"];
    if (!sections || !sections.IsSequence()) {
        throw std::runtime_error("Message rule [" + rule.id + "] has no sections");
    }

    for (const auto& section_node : sections) {
        rule.sections.push_back(parse_section_node(section_node, rule, rulesets));
    }

    std::vector<IdentificationExample> examples;
    const YAML::Node example_nodes = node["examples"];
    if (example_nodes && example_nodes.IsSequence()) {
        for (const auto& example_node : example_nodes) {
            examples.push_back(parse_identification_example(example_node, definition_dir));
        }
    }
    validate_identification_examples(rule, examples);

    return rule;
}

void load_parser_rules_file(const std::filesystem::path& file,
    std::unordered_map<std::string, ParserRuleSet>& rulesets) {
    YAML::Node root = YAML::LoadFile(file.string());
    const YAML::Node rules = root["rules"];
    if (!rules || !rules.IsSequence()) {
        return;
    }

    for (const auto& rule_node : rules) {
        ParserRuleSet ruleset = parse_ruleset(rule_node, file.parent_path());
        validate_ruleset_examples(ruleset);
        rulesets[ruleset.name] = std::move(ruleset);
    }
}

void load_identification_file(const std::filesystem::path& file,
    const std::unordered_map<std::string, ParserRuleSet>& rulesets,
    std::vector<MessageRule>& rules) {
    YAML::Node root = YAML::LoadFile(file.string());
    const YAML::Node messages = root["messages"];
    if (!messages || !messages.IsSequence()) {
        return;
    }

    for (const auto& message_node : messages) {
        rules.push_back(parse_message_rule(message_node, rulesets, file.parent_path()));
    }
}

void collect_example_messages_from_file(const std::filesystem::path& file,
    std::unordered_set<std::string>& seen,
    std::vector<std::string>& messages) {
    YAML::Node root = YAML::LoadFile(file.string());
    const YAML::Node definitions = root["messages"];
    if (!definitions || !definitions.IsSequence()) {
        return;
    }

    for (const auto& message_node : definitions) {
        const YAML::Node examples = message_node["examples"];
        if (!examples || !examples.IsSequence()) {
            continue;
        }

        for (const auto& example : examples) {
            const std::string message = load_example_message(example, file.parent_path());
            if (seen.insert(message).second) {
                messages.push_back(message);
            }
        }
    }
}

template <typename FileHandler>
void for_each_rule_file(const std::filesystem::path& input, FileHandler&& handler) {
    namespace fs = std::filesystem;

    if (fs::is_regular_file(input)) {
        handler(input);
        return;
    }

    if (!fs::is_directory(input)) {
        throw std::runtime_error("Rules path is not a file or directory [" + input.string() + "]");
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(input)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const fs::path extension = entry.path().extension();
        if (extension == ".yaml" || extension == ".yml") {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());
    for (const auto& file : files) {
        handler(file);
    }
}

} // namespace

std::vector<MessageRule> load_rules(const std::string& path) {
    namespace fs = std::filesystem;

    const fs::path input(path);
    if (!fs::exists(input)) {
        throw std::runtime_error("Rules path does not exist [" + path + "]");
    }

    std::unordered_map<std::string, ParserRuleSet> rulesets;
    std::vector<MessageRule> rules;

    for_each_rule_file(input, [&](const fs::path& file) {
        load_parser_rules_file(file, rulesets);
    });

    for_each_rule_file(input, [&](const fs::path& file) {
        load_identification_file(file, rulesets, rules);
    });

    if (rules.empty()) {
        throw std::runtime_error("No identification rules were loaded from [" + path + "]");
    }

    return rules;
}

std::vector<std::string> load_example_messages(const std::string& path) {
    namespace fs = std::filesystem;

    const fs::path input(path);
    if (!fs::exists(input)) {
        throw std::runtime_error("Rules path does not exist [" + path + "]");
    }

    std::unordered_set<std::string> seen;
    std::vector<std::string> messages;
    for_each_rule_file(input, [&](const fs::path& file) {
        collect_example_messages_from_file(file, seen, messages);
    });

    return messages;
}

} // namespace RuleLoader
} // namespace parser_framework
