#ifndef PARSER_FRAMEWORK_PARSING_ENGINES_HPP
#define PARSER_FRAMEWORK_PARSING_ENGINES_HPP

#include <cstddef>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "parser_framework/ParserFramework.hpp"

namespace parser_framework {

struct SectionSlice {
    std::string text;
    std::size_t start {0};
    std::size_t end {0};
};

struct FilterMatch {
    bool matched {false};
    std::map<std::string, std::string> captures;
};

class RegexParsingEngine {
public:
    FilterMatch match_message_filter(
        const MessageFilterRule& filter,
        const std::string& message,
        std::vector<std::string>& errors) const;

    std::vector<SectionSlice> locate_slices(
        const std::string& text,
        std::size_t base_offset,
        const SectionRule& section,
        std::vector<std::string>& errors) const;

    bool parse_section(
        const SectionRule& section,
        const SectionSlice& slice,
        ParseResult& result) const;

    std::unordered_map<std::string, std::string> parse_values(
        const std::vector<RegexTokenRule>& token_rules,
        const std::string& message,
        std::vector<std::string>& errors) const;
};

class KVParsingEngine {
public:
    bool parse_section(
        const SectionRule& section,
        const SectionSlice& slice,
        ParseResult& result) const;

    std::unordered_map<std::string, std::string> parse_values(
        const std::vector<DeclaredToken>& declared_tokens,
        const std::string& message) const;
};

class JSONParsingEngine {
public:
    bool parse_section(
        const SectionRule& section,
        const SectionSlice& slice,
        ParseResult& result) const;

    std::unordered_map<std::string, std::string> parse_values(
        const SectionRule& section,
        const std::string& message,
        std::vector<std::string>& errors) const;
};

} // namespace parser_framework

#endif
