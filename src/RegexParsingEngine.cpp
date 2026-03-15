#include "parser_framework/ParsingEngines.hpp"

#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Framework/Match.h"
#include "Framework/Regex.h"

namespace parser_framework {

namespace {

struct CaptureMatch {
    std::string value;
    std::size_t start {0};
    std::size_t end {0};
};

std::string rewrite_pattern_for_capture(const std::string& pattern, std::size_t target_group) {
    std::string rewritten;
    rewritten.reserve(pattern.size() + 4);

    bool escaped = false;
    bool in_char_class = false;
    std::size_t capture_index = 0;

    for (std::size_t i = 0; i < pattern.size(); ++i) {
        const char ch = pattern[i];

        if (escaped) {
            rewritten.push_back(ch);
            escaped = false;
            continue;
        }

        if (ch == '\\') {
            rewritten.push_back(ch);
            escaped = true;
            continue;
        }

        if (in_char_class) {
            rewritten.push_back(ch);
            if (ch == ']') {
                in_char_class = false;
            }
            continue;
        }

        if (ch == '[') {
            rewritten.push_back(ch);
            in_char_class = true;
            continue;
        }

        if (ch != '(') {
            rewritten.push_back(ch);
            continue;
        }

        const bool has_question = i + 1 < pattern.size() && pattern[i + 1] == '?';
        const bool named_capture = has_question && i + 2 < pattern.size() && pattern[i + 2] == '<' &&
            !(i + 3 < pattern.size() && (pattern[i + 3] == '=' || pattern[i + 3] == '!'));

        if (named_capture) {
            const std::size_t name_end = pattern.find('>', i + 3);
            if (name_end == std::string::npos) {
                throw std::runtime_error("Invalid named capture group in pattern [" + pattern + "]");
            }

            ++capture_index;
            rewritten.append(target_group == 0 || capture_index != target_group ? "(?:" : "(");
            i = name_end;
            continue;
        }

        if (has_question) {
            rewritten.push_back(ch);
            continue;
        }

        ++capture_index;
        rewritten.append(target_group == 0 || capture_index != target_group ? "(?:" : "(");
    }

    if (target_group > 0 && capture_index < target_group) {
        std::ostringstream stream;
        stream << "Pattern [" << pattern << "] does not contain capture group " << target_group;
        throw std::runtime_error(stream.str());
    }

    if (target_group == 0) {
        return "(" + rewritten + ")";
    }

    return rewritten;
}

std::optional<CaptureMatch> search_capture(const std::string& text,
    const std::string& pattern,
    std::size_t group,
    Match::Type type,
    std::size_t offset = 0) {
    const std::string rewritten = rewrite_pattern_for_capture(pattern, group);
    Match match(text);
    match.set_match_type(type);
    match.offset(offset);

    Regex regex(rewritten);
    if (!regex.match(match)) {
        return std::nullopt;
    }

    const std::string value = regex.as_string();
    const std::size_t end = match.offset();
    const std::size_t start = end >= value.size() ? end - value.size() : 0;

    return CaptureMatch{value, start, end};
}

} // namespace

FilterMatch RegexParsingEngine::match_message_filter(
    const MessageFilterRule& filter,
    const std::string& message,
    std::vector<std::string>& errors) const {
    FilterMatch result;

    try {
        const std::optional<CaptureMatch> whole_match =
            search_capture(message, filter.pattern, 0, Match::Type::SEQUENTIAL);
        if (!whole_match.has_value()) {
            return result;
        }

        result.matched = true;
        for (const auto& capture : filter.captures) {
            const std::optional<CaptureMatch> capture_match =
                search_capture(message, filter.pattern, capture.group, Match::Type::SEQUENTIAL);
            if (capture_match.has_value()) {
                result.captures[capture.name] = capture_match->value;
            }
        }
    } catch (const std::exception& ex) {
        errors.push_back("Invalid message filter regex [" + filter.pattern + "]: " + ex.what());
    }

    return result;
}

std::vector<SectionSlice> RegexParsingEngine::locate_slices(
    const std::string& text,
    std::size_t base_offset,
    const SectionRule& section,
    std::vector<std::string>& errors) const {
    std::vector<SectionSlice> slices;

    if (!section.locator_pattern.has_value()) {
        slices.push_back(SectionSlice{text, base_offset, base_offset + text.size()});
        return slices;
    }

    try {
        if (!section.allow_multiple) {
            const std::optional<CaptureMatch> match =
                search_capture(text, *section.locator_pattern, section.locator_group, Match::Type::SEQUENTIAL);
            if (match.has_value()) {
                slices.push_back(SectionSlice{
                    match->value,
                    base_offset + match->start,
                    base_offset + match->end
                });
            }
            return slices;
        }

        std::size_t offset = 0;
        while (offset <= text.size()) {
            const std::optional<CaptureMatch> match =
                search_capture(text, *section.locator_pattern, section.locator_group, Match::Type::SEQUENTIAL, offset);
            if (!match.has_value()) {
                break;
            }

            slices.push_back(SectionSlice{
                match->value,
                base_offset + match->start,
                base_offset + match->end
            });

            if (match->end <= offset) {
                break;
            }
            offset = match->end;
        }
    } catch (const std::exception& ex) {
        errors.push_back("Invalid locator regex for section [" + section.id + "]: " + ex.what());
    }

    return slices;
}

bool RegexParsingEngine::parse_section(
    const SectionRule& section,
    const SectionSlice& slice,
    ParseResult& result) const {
    bool matched = false;

    for (const auto& token_rule : section.regex_tokens) {
        try {
            const std::optional<CaptureMatch> capture =
                search_capture(slice.text, token_rule.pattern, token_rule.group, Match::Type::RANDOM);
            if (!capture.has_value()) {
                continue;
            }

            result.tokens.push_back(Token{
                token_rule.name,
                capture->value,
                section.id,
                "regex-parser",
                {},
                slice.start + capture->start,
                slice.start + capture->end
            });
            matched = true;
        } catch (const std::exception& ex) {
            result.errors.push_back("Invalid token regex for section [" + section.id + "]: " + ex.what());
        }
    }

    return matched;
}

std::unordered_map<std::string, std::string> RegexParsingEngine::parse_values(
    const std::vector<RegexTokenRule>& token_rules,
    const std::string& message,
    std::vector<std::string>& errors) const {
    std::unordered_map<std::string, std::string> values;

    for (const auto& token_rule : token_rules) {
        try {
            const std::optional<CaptureMatch> capture =
                search_capture(message, token_rule.pattern, token_rule.group, Match::Type::RANDOM);
            if (capture.has_value()) {
                values[token_rule.name] = capture->value;
            }
        } catch (const std::exception& ex) {
            errors.push_back("Invalid regex token [" + token_rule.name + "]: " + ex.what());
        }
    }

    return values;
}

} // namespace parser_framework
