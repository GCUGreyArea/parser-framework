#include "parser_framework/RuleLoader.hpp"

#include <filesystem>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace parser_framework {
namespace RuleLoader {

namespace {

struct KvRuleSet {
    std::string id;
    std::string name;
    std::unordered_set<std::string> bindings;
    std::vector<DeclaredToken> tokens;
    YAML::Node examples;
};

struct IdentificationExample {
    std::string message;
    std::unordered_map<std::string, std::string> captures;
};

std::string first_non_empty_capture(const std::smatch& match, std::size_t begin) {
    for (std::size_t i = begin; i < match.size(); ++i) {
        if (match[i].matched) {
            return match[i].str();
        }
    }

    return {};
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

std::unordered_map<std::string, std::string> match_identification_filter(
    const std::vector<MessageFilterRule>& filters,
    const std::string& message) {
    for (const auto& filter : filters) {
        std::regex compiled(filter.pattern);
        std::smatch match;
        if (!std::regex_search(message, match, compiled)) {
            continue;
        }

        std::unordered_map<std::string, std::string> captures;
        for (const auto& capture : filter.captures) {
            if (capture.group < match.size() && match[capture.group].matched) {
                captures.emplace(capture.name, match[capture.group].str());
            }
        }

        return captures;
    }

    return {};
}

std::unordered_map<std::string, std::string> parse_kv_values(
    const std::string& message,
    const std::vector<DeclaredToken>& declared_tokens) {
    static const std::regex kv_regex(
        R"KV(([A-Za-z0-9_.-]+)\s*[:=]\s*(?:"([^"]*)"|'([^']*)'|\[([^\]]*)\]|([^;\s\]]+)))KV");

    std::unordered_set<std::string> declared_names;
    for (const auto& token : declared_tokens) {
        declared_names.insert(token.name);
    }

    std::unordered_map<std::string, std::string> values;
    for (std::sregex_iterator it(message.begin(), message.end(), kv_regex), end; it != end; ++it) {
        const std::smatch& match = *it;
        const std::string name = match[1].str();
        if (!declared_names.empty() && declared_names.find(name) == declared_names.end()) {
            continue;
        }

        values[name] = first_non_empty_capture(match, 2);
    }

    return values;
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

KvRuleSet parse_kv_ruleset(const YAML::Node& node) {
    KvRuleSet ruleset;
    ruleset.id = node["id"].as<std::string>();
    ruleset.name = node["name"].as<std::string>();
    ruleset.tokens = parse_declared_tokens(node["tokens"]);

    if (node["bindings"]) {
        for (const auto& binding : node["bindings"]) {
            ruleset.bindings.insert(binding.as<std::string>());
        }
    }

    ruleset.examples = node["examples"];
    return ruleset;
}

void validate_kv_ruleset_examples(const KvRuleSet& ruleset) {
    if (!ruleset.examples || !ruleset.examples.IsSequence()) {
        return;
    }

    for (const auto& example : ruleset.examples) {
        const std::string message = example["message"].as<std::string>();
        const std::unordered_map<std::string, std::string> parsed = parse_kv_values(message, ruleset.tokens);
        const YAML::Node expected_tokens = example["expect"]["tokens"];
        if (expected_tokens) {
            for (YAML::const_iterator it = expected_tokens.begin(); it != expected_tokens.end(); ++it) {
                const std::string name = it->first.as<std::string>();
                const std::string expected = it->second.as<std::string>();
                auto parsed_it = parsed.find(name);
                if (parsed_it == parsed.end()) {
                    throw std::runtime_error("KV example for ruleset [" + ruleset.name + "] missing token [" + name + "]");
                }
                if (parsed_it->second != expected) {
                    throw std::runtime_error(
                        "KV example for ruleset [" + ruleset.name + "] token [" + name + "] expected [" +
                        expected + "] but parsed [" + parsed_it->second + "]");
                }
            }
        }

        const YAML::Node absent_tokens = example["expect"]["absent_tokens"];
        if (absent_tokens) {
            for (const auto& absent : absent_tokens) {
                const std::string name = absent.as<std::string>();
                if (parsed.find(name) != parsed.end()) {
                    throw std::runtime_error(
                        "KV example for ruleset [" + ruleset.name + "] unexpectedly parsed absent token [" + name + "]");
                }
            }
        }
    }
}

IdentificationExample parse_identification_example(const YAML::Node& node) {
    IdentificationExample example;
    example.message = node["message"].as<std::string>();

    const YAML::Node captures = node["expect"]["captures"];
    if (captures && captures.IsMap()) {
        for (YAML::const_iterator it = captures.begin(); it != captures.end(); ++it) {
            example.captures.emplace(it->first.as<std::string>(), it->second.as<std::string>());
        }
    }

    return example;
}

void validate_identification_examples(const MessageRule& rule, const std::vector<IdentificationExample>& examples) {
    for (const auto& example : examples) {
        const std::unordered_map<std::string, std::string> captures =
            match_identification_filter(rule.filters, example.message);
        if (captures.empty()) {
            throw std::runtime_error("Identification example for rule [" + rule.name + "] did not match");
        }

        for (const auto& expected : example.captures) {
            auto capture_it = captures.find(expected.first);
            if (capture_it == captures.end()) {
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

void load_parser_rules_file(const std::filesystem::path& file,
    std::unordered_map<std::string, KvRuleSet>& kv_rulesets) {
    YAML::Node root = YAML::LoadFile(file.string());
    const YAML::Node rules = root["rules"];
    if (!rules || !rules.IsSequence()) {
        return;
    }

    for (const auto& rule_node : rules) {
        const std::string parser = rule_node["parser"].as<std::string>();
        if (parser == "kv") {
            KvRuleSet ruleset = parse_kv_ruleset(rule_node);
            validate_kv_ruleset_examples(ruleset);
            kv_rulesets[ruleset.name] = ruleset;
            continue;
        }

        throw std::runtime_error("Unsupported parser ruleset type [" + parser + "]");
    }
}

SectionRule parse_identification_section(const YAML::Node& node,
    const MessageRule& rule,
    const std::unordered_map<std::string, KvRuleSet>& kv_rulesets) {
    SectionRule section;
    section.id = node["extract"].as<std::string>();
    section.extract_name = node["extract"].as<std::string>();
    section.ruleset_name = node["ruleset"].as<std::string>();

    const std::string parser = node["parser"].as<std::string>();
    if (parser == "kv") {
        section.kind = SectionKind::KV;
        auto kv_it = kv_rulesets.find(section.ruleset_name);
        if (kv_it == kv_rulesets.end()) {
            throw std::runtime_error("Unknown kv ruleset [" + section.ruleset_name + "]");
        }

        if (!kv_it->second.bindings.empty() && kv_it->second.bindings.find(rule.id) == kv_it->second.bindings.end()) {
            throw std::runtime_error(
                "KV ruleset [" + section.ruleset_name + "] is not bound to identification rule [" + rule.id + "]");
        }

        section.declared_tokens = kv_it->second.tokens;
        return section;
    }

    throw std::runtime_error("Unsupported section parser [" + parser + "]");
}

MessageRule parse_message_rule(const YAML::Node& node,
    const std::unordered_map<std::string, KvRuleSet>& kv_rulesets) {
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
        rule.sections.push_back(parse_identification_section(section_node, rule, kv_rulesets));
    }

    std::vector<IdentificationExample> examples;
    const YAML::Node example_nodes = node["examples"];
    if (example_nodes && example_nodes.IsSequence()) {
        for (const auto& example_node : example_nodes) {
            examples.push_back(parse_identification_example(example_node));
        }
    }
    validate_identification_examples(rule, examples);

    return rule;
}

void load_identification_file(const std::filesystem::path& file,
    const std::unordered_map<std::string, KvRuleSet>& kv_rulesets,
    std::vector<MessageRule>& rules) {
    YAML::Node root = YAML::LoadFile(file.string());
    const YAML::Node messages = root["messages"];
    if (!messages || !messages.IsSequence()) {
        return;
    }

    for (const auto& message_node : messages) {
        rules.push_back(parse_message_rule(message_node, kv_rulesets));
    }
}

} // namespace

std::vector<MessageRule> load_rules(const std::string& path) {
    namespace fs = std::filesystem;

    const fs::path input(path);
    if (!fs::exists(input)) {
        throw std::runtime_error("Rules path does not exist [" + path + "]");
    }

    std::unordered_map<std::string, KvRuleSet> kv_rulesets;
    std::vector<MessageRule> rules;

    auto load_file = [&](const fs::path& file) {
        load_parser_rules_file(file, kv_rulesets);
    };

    if (fs::is_regular_file(input)) {
        load_file(input);
        load_identification_file(input, kv_rulesets, rules);
        return rules;
    }

    if (!fs::is_directory(input)) {
        throw std::runtime_error("Rules path is not a file or directory [" + path + "]");
    }

    for (const auto& entry : fs::recursive_directory_iterator(input)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".yaml" || entry.path().extension() == ".yml")) {
            load_file(entry.path());
        }
    }

    for (const auto& entry : fs::recursive_directory_iterator(input)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".yaml" || entry.path().extension() == ".yml")) {
            load_identification_file(entry.path(), kv_rulesets, rules);
        }
    }

    if (rules.empty()) {
        throw std::runtime_error("No identification rules were loaded from [" + path + "]");
    }

    return rules;
}

} // namespace RuleLoader
} // namespace parser_framework
