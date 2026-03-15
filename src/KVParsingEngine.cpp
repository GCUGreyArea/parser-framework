#include "parser_framework/ParsingEngines.hpp"

#include <regex>
#include <string>
#include <unordered_map>

namespace parser_framework {

namespace {

std::string first_non_empty_capture(const std::smatch& match, std::size_t begin) {
    for (std::size_t i = begin; i < match.size(); ++i) {
        if (match[i].matched) {
            return match[i].str();
        }
    }

    return {};
}

} // namespace

bool KVParsingEngine::parse_section(
    const SectionRule& section,
    const SectionSlice& slice,
    ParseResult& result) const {
    static const std::regex kv_regex(
        R"KV(([A-Za-z0-9_.-]+)\s*[:=]\s*(?:"([^"]*)"|'([^']*)'|\[([^\]]*)\]|([^;\s\]]+)))KV");

    std::unordered_map<std::string, std::string> declared;
    for (const auto& token : section.declared_tokens) {
        declared.emplace(token.name, token.type);
    }

    bool matched = false;
    for (std::sregex_iterator it(slice.text.begin(), slice.text.end(), kv_regex), end; it != end; ++it) {
        const std::smatch& match = *it;
        const std::string token_name = match[1].str();
        if (!declared.empty() && declared.find(token_name) == declared.end()) {
            continue;
        }

        const std::string value = first_non_empty_capture(match, 2);
        const std::size_t local_start = static_cast<std::size_t>(match.position(0));
        const std::size_t local_end = local_start + static_cast<std::size_t>(match.length(0));

        result.tokens.push_back(Token{
            token_name,
            value,
            section.id,
            "kv",
            token_name,
            slice.start + local_start,
            slice.start + local_end
        });
        matched = true;
    }

    return matched;
}

std::unordered_map<std::string, std::string> KVParsingEngine::parse_values(
    const std::vector<DeclaredToken>& declared_tokens,
    const std::string& message) const {
    static const std::regex kv_regex(
        R"KV(([A-Za-z0-9_.-]+)\s*[:=]\s*(?:"([^"]*)"|'([^']*)'|\[([^\]]*)\]|([^;\s\]]+)))KV");

    std::unordered_map<std::string, std::string> declared;
    for (const auto& token : declared_tokens) {
        declared.emplace(token.name, token.type);
    }

    std::unordered_map<std::string, std::string> values;
    for (std::sregex_iterator it(message.begin(), message.end(), kv_regex), end; it != end; ++it) {
        const std::smatch& match = *it;
        const std::string token_name = match[1].str();
        if (!declared.empty() && declared.find(token_name) == declared.end()) {
            continue;
        }

        values[token_name] = first_non_empty_capture(match, 2);
    }

    return values;
}

} // namespace parser_framework
