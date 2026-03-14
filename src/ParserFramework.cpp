#include "parser_framework/ParserFramework.hpp"

#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "jqpath.h"
#include "jsmn.hpp"

namespace parser_framework {

namespace {

struct SectionSlice {
    std::string text;
    std::size_t start {0};
    std::size_t end {0};
};

std::string first_non_empty_capture(const std::smatch& match, std::size_t begin) {
    for (std::size_t i = begin; i < match.size(); ++i) {
        if (match[i].matched) {
            return match[i].str();
        }
    }

    return {};
}

std::string json_token_to_string(const std::string& json, const jsmntok_t* token) {
    if (token == nullptr || token->start < 0 || token->end < token->start) {
        return {};
    }

    return json.substr(static_cast<std::size_t>(token->start),
        static_cast<std::size_t>(token->end - token->start));
}

std::vector<SectionSlice> locate_slices(const std::string& text, std::size_t base_offset,
    const SectionRule& section, std::vector<std::string>& errors) {
    std::vector<SectionSlice> slices;

    if (!section.locator_pattern.has_value()) {
        slices.push_back(SectionSlice{text, base_offset, base_offset + text.size()});
        return slices;
    }

    try {
        std::regex locator(*section.locator_pattern);

        if (section.allow_multiple) {
            for (std::sregex_iterator it(text.begin(), text.end(), locator), end; it != end; ++it) {
                const std::smatch& match = *it;
                if (section.locator_group >= match.size() || !match[section.locator_group].matched) {
                    continue;
                }

                const std::size_t local_start = static_cast<std::size_t>(match.position(section.locator_group));
                const std::size_t local_length = static_cast<std::size_t>(match.length(section.locator_group));
                slices.push_back(SectionSlice{
                    match[section.locator_group].str(),
                    base_offset + local_start,
                    base_offset + local_start + local_length
                });
            }
        } else {
            std::smatch match;
            if (std::regex_search(text, match, locator)) {
                if (section.locator_group < match.size() && match[section.locator_group].matched) {
                    const std::size_t local_start = static_cast<std::size_t>(match.position(section.locator_group));
                    const std::size_t local_length = static_cast<std::size_t>(match.length(section.locator_group));
                    slices.push_back(SectionSlice{
                        match[section.locator_group].str(),
                        base_offset + local_start,
                        base_offset + local_start + local_length
                    });
                }
            }
        }
    } catch (const std::regex_error& ex) {
        errors.push_back("Invalid locator regex for section [" + section.id + "]: " + ex.what());
    }

    return slices;
}

void parse_regex_tokens(const SectionRule& section, const SectionSlice& slice, ParseResult& result) {
    for (const auto& token_rule : section.regex_tokens) {
        try {
            std::regex regex(token_rule.pattern);
            std::smatch match;

            if (!std::regex_search(slice.text, match, regex)) {
                continue;
            }

            if (token_rule.group >= match.size() || !match[token_rule.group].matched) {
                continue;
            }

            const std::size_t local_start = static_cast<std::size_t>(match.position(token_rule.group));
            const std::size_t local_length = static_cast<std::size_t>(match.length(token_rule.group));

            result.tokens.push_back(Token{
                token_rule.name,
                match[token_rule.group].str(),
                section.id,
                "regex",
                {},
                slice.start + local_start,
                slice.start + local_start + local_length
            });
        } catch (const std::regex_error& ex) {
            result.errors.push_back("Invalid token regex for section [" + section.id + "]: " + ex.what());
        }
    }
}

void parse_kv_tokens(const SectionRule& section, const SectionSlice& slice, ParseResult& result) {
    static const std::regex kv_regex(
        R"KV(([A-Za-z0-9_.-]+)\s*[:=]\s*(?:"([^"]*)"|'([^']*)'|\[([^\]]*)\]|([^;\s\]]+)))KV");

    for (std::sregex_iterator it(slice.text.begin(), slice.text.end(), kv_regex), end; it != end; ++it) {
        const std::smatch& match = *it;
        const std::string value = first_non_empty_capture(match, 2);
        const std::size_t local_start = static_cast<std::size_t>(match.position(0));
        const std::size_t local_end = local_start + static_cast<std::size_t>(match.length(0));

        result.tokens.push_back(Token{
            match[1].str(),
            value,
            section.id,
            "kv",
            match[1].str(),
            slice.start + local_start,
            slice.start + local_end
        });
    }

    parse_regex_tokens(section, slice, result);
}

bool json_match_succeeds(const SectionRule& section, jsmn_parser& parser, const std::string& json,
    ParseResult& result) {
    for (const auto& match_rule : section.json_matches) {
        struct jqpath* path = jsonpath_parse_string(match_rule.path.c_str());
        if (path == nullptr) {
            result.errors.push_back("Failed to parse JSONPath [" + match_rule.path + "]");
            return false;
        }

        JQ* value = parser.get_path(path);
        const jsmntok_t* token = value == nullptr ? nullptr : value->get_token();
        const std::string rendered = json_token_to_string(json, token);
        jqpath_close_path(path);

        if (token == nullptr) {
            return false;
        }

        if (match_rule.equals.has_value() && rendered != *match_rule.equals) {
            return false;
        }
    }

    return true;
}

void parse_json_tokens(const SectionRule& section, const SectionSlice& slice, ParseResult& result) {
    jsmn_parser parser(slice.text.c_str(), 2);
    const int parse_ret = parser.parse();
    if (parse_ret < 0) {
        std::ostringstream stream;
        stream << "JSON parse failed in section [" << section.id << "] with error " << parse_ret;
        result.errors.push_back(stream.str());
        return;
    }

    if (!json_match_succeeds(section, parser, slice.text, result)) {
        return;
    }

    for (const auto& token_rule : section.json_tokens) {
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
        }

        jqpath_close_path(path);
    }
}

void parse_section(const SectionRule& section, const SectionSlice& parent_slice, ParseResult& result) {
    const std::vector<SectionSlice> slices = locate_slices(parent_slice.text, parent_slice.start, section, result.errors);
    for (const auto& slice : slices) {
        switch (section.kind) {
        case SectionKind::FREE_TEXT:
            parse_regex_tokens(section, slice, result);
            break;
        case SectionKind::KV:
            parse_kv_tokens(section, slice, result);
            break;
        case SectionKind::JSON:
            parse_json_tokens(section, slice, result);
            break;
        case SectionKind::MIXED:
            for (const auto& child : section.children) {
                parse_section(child, slice, result);
            }
            break;
        }
    }
}

bool message_matches(const MessageRule& rule, const std::string& message) {
    for (const auto& anchor_pattern : rule.anchor_patterns) {
        try {
            std::regex anchor(anchor_pattern);
            if (std::regex_search(message, anchor)) {
                return true;
            }
        } catch (const std::regex_error&) {
            return false;
        }
    }

    return false;
}

} // namespace

ParserFramework::ParserFramework(std::vector<MessageRule> message_rules)
    : m_message_rules(std::move(message_rules)) {}

void ParserFramework::add_message_rule(MessageRule message_rule) {
    m_message_rules.push_back(std::move(message_rule));
}

ParseResult ParserFramework::parse_message(const std::string& message) const {
    ParseResult result;

    for (const auto& rule : m_message_rules) {
        if (!message_matches(rule, message)) {
            continue;
        }

        result.matched = true;
        result.message_rule_id = rule.id;
        result.message_rule_name = rule.name;

        const SectionSlice root {message, 0, message.size()};
        for (const auto& section : rule.sections) {
            parse_section(section, root, result);
        }

        return result;
    }

    result.errors.push_back("No message rule matched");
    return result;
}

} // namespace parser_framework
