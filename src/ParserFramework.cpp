#include "parser_framework/ParserFramework.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "parser_framework/DynamicPropertyEngine.hpp"
#include "parser_framework/ParsingEngines.hpp"

namespace parser_framework {

namespace {

struct RenderNode {
    std::optional<std::string> value;
    std::vector<std::string> child_order;
    std::unordered_map<std::string, std::unique_ptr<RenderNode>> children;
};

std::string escape_json_string(const std::string& value) {
    std::ostringstream stream;
    for (const char ch : value) {
        switch (ch) {
        case '\\':
            stream << "\\\\";
            break;
        case '"':
            stream << "\\\"";
            break;
        case '\b':
            stream << "\\b";
            break;
        case '\f':
            stream << "\\f";
            break;
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        default:
            stream << ch;
            break;
        }
    }

    return stream.str();
}

void append_indent(std::ostringstream& stream, std::size_t depth) {
    for (std::size_t i = 0; i < depth; ++i) {
        stream << "  ";
    }
}

void record_matched_rule_id(ParseResult& result, const std::string& rule_id) {
    if (rule_id.empty()) {
        return;
    }

    if (std::find(result.token_extraction.begin(), result.token_extraction.end(), rule_id) ==
        result.token_extraction.end()) {
        result.token_extraction.push_back(rule_id);
    }
}

void remember_child_order(RenderNode& node, const std::string& child_name) {
    if (node.children.find(child_name) == node.children.end()) {
        node.child_order.push_back(child_name);
    }
}

void insert_render_token(RenderNode& root,
    const std::string& token_name,
    const std::string& token_value) {
    std::size_t start = 0;
    RenderNode* current = &root;

    while (start != std::string::npos) {
        const std::size_t dot = token_name.find('.', start);
        const std::string segment = token_name.substr(start,
            dot == std::string::npos ? std::string::npos : dot - start);
        remember_child_order(*current, segment);
        std::unique_ptr<RenderNode>& child = current->children[segment];
        if (!child) {
            child = std::make_unique<RenderNode>();
        }
        current = child.get();
        if (dot == std::string::npos) {
            current->value = token_value;
            return;
        }
        start = dot + 1;
    }
}

void render_node_as_json(std::ostringstream& stream, const RenderNode& node, std::size_t depth);

void render_object_children(std::ostringstream& stream, const RenderNode& node, std::size_t depth) {
    bool first_entry = true;

    if (node.value.has_value()) {
        append_indent(stream, depth);
        stream << "\"raw\": \"" << escape_json_string(*node.value) << "\"";
        first_entry = false;
    }

    for (const auto& child_name : node.child_order) {
        auto child_it = node.children.find(child_name);
        if (child_it == node.children.end()) {
            continue;
        }

        if (!first_entry) {
            stream << ",\n";
        }
        append_indent(stream, depth);
        stream << "\"" << escape_json_string(child_name) << "\": ";
        render_node_as_json(stream, *child_it->second, depth);
        first_entry = false;
    }

    if (!first_entry) {
        stream << "\n";
    }
}

void render_node_as_json(std::ostringstream& stream, const RenderNode& node, std::size_t depth) {
    if (node.children.empty()) {
        stream << "\"" << escape_json_string(node.value.value_or(std::string{})) << "\"";
        return;
    }

    stream << "{\n";
    render_object_children(stream, node, depth + 1);
    append_indent(stream, depth);
    stream << "}";
}

RenderNode build_render_tree(const ParseResult& result) {
    RenderNode root;
    std::unordered_set<std::string> emitted_names;
    for (const auto& token : result.tokens) {
        if (!emitted_names.insert(token.name).second) {
            continue;
        }
        insert_render_token(root, token.name, token.value);
    }

    return root;
}

std::vector<SectionSlice> resolve_section_slices(const std::string& message,
    const SectionRule& section,
    const std::map<std::string, std::string>& captures,
    std::vector<std::string>& errors,
    const RegexParsingEngine& regex_engine) {
    auto capture_it = captures.find(section.extract_name);
    if (capture_it != captures.end()) {
        const std::string& slice = capture_it->second;
        const std::size_t start = message.find(slice);
        const std::size_t resolved_start = start == std::string::npos ? 0 : start;
        return {SectionSlice{slice, resolved_start, resolved_start + slice.size()}};
    }

    if (!section.extract_name.empty()) {
        errors.push_back("Missing extracted section [" + section.extract_name + "]");
        return {};
    }

    return regex_engine.locate_slices(message, 0, section, errors);
}

bool parse_section_slices(const SectionRule& section,
    const std::vector<SectionSlice>& slices,
    ParseResult& result,
    const RegexParsingEngine& regex_engine,
    const KVParsingEngine& kv_engine,
    const JSONParsingEngine& json_engine,
    const DynamicPropertyEngine& property_engine);

bool dispatch_section(const SectionRule& section,
    const SectionSlice& slice,
    ParseResult& result,
    const RegexParsingEngine& regex_engine,
    const KVParsingEngine& kv_engine,
    const JSONParsingEngine& json_engine,
    const DynamicPropertyEngine& property_engine) {
    switch (section.kind) {
    case SectionKind::FREE_TEXT:
        return regex_engine.parse_section(section, slice, result);
    case SectionKind::KV:
        return kv_engine.parse_section(section, slice, result);
    case SectionKind::JSON:
        return json_engine.parse_section(section, slice, result);
    case SectionKind::MIXED: {
        bool matched = false;
        for (const auto& child : section.children) {
            const std::vector<SectionSlice> child_slices =
                regex_engine.locate_slices(slice.text, slice.start, child, result.errors);
            matched = parse_section_slices(
                child,
                child_slices,
                result,
                regex_engine,
                kv_engine,
                json_engine,
                property_engine) ||
                matched;
        }
        return matched;
    }
    }

    return false;
}

bool parse_section_slices(const SectionRule& section,
    const std::vector<SectionSlice>& slices,
    ParseResult& result,
    const RegexParsingEngine& regex_engine,
    const KVParsingEngine& kv_engine,
    const JSONParsingEngine& json_engine,
    const DynamicPropertyEngine& property_engine) {
    bool matched = false;

    for (const auto& slice : slices) {
        matched = dispatch_section(section, slice, result, regex_engine, kv_engine, json_engine, property_engine) ||
            matched;
    }

    if (matched) {
        record_matched_rule_id(result, section.ruleset_id);
        const std::map<std::string, std::string> properties =
            property_engine.evaluate(section.dynamic_properties, result, result.errors);
        result.properties.insert(properties.begin(), properties.end());
    }

    return matched;
}

FilterMatch message_matches(const MessageRule& rule,
    const std::string& message,
    const RegexParsingEngine& regex_engine,
    std::vector<std::string>& errors) {
    for (const auto& filter : rule.filters) {
        FilterMatch match = regex_engine.match_message_filter(filter, message, errors);
        if (match.matched) {
            return match;
        }
    }

    return {};
}

} // namespace

ParserFramework::ParserFramework(std::vector<MessageRule> message_rules)
    : m_message_rules(std::move(message_rules)) {}

void ParserFramework::add_message_rule(MessageRule message_rule) {
    m_message_rules.push_back(std::move(message_rule));
}

ParseResult ParserFramework::parse_message(const std::string& message) const {
    ParseResult result;
    const RegexParsingEngine regex_engine;
    const KVParsingEngine kv_engine;
    const JSONParsingEngine json_engine;
    const DynamicPropertyEngine property_engine;

    for (const auto& rule : m_message_rules) {
        FilterMatch match = message_matches(rule, message, regex_engine, result.errors);
        if (!match.matched) {
            continue;
        }

        result.matched = true;
        result.message_rule_id = rule.id;
        result.message_rule_name = rule.name;
        result.event_name = rule.name;
        result.event_pattern_id = rule.id;
        record_matched_rule_id(result, rule.id);

        for (const auto& section : rule.sections) {
            const std::vector<SectionSlice> slices =
                resolve_section_slices(message, section, match.captures, result.errors, regex_engine);
            parse_section_slices(section, slices, result, regex_engine, kv_engine, json_engine, property_engine);
        }

        return result;
    }

    result.errors.push_back("No message rule matched");
    return result;
}

std::vector<ParseResult> ParserFramework::parse_messages(const std::vector<std::string>& messages) const {
    std::vector<ParseResult> results;
    results.reserve(messages.size());

    for (const auto& message : messages) {
        results.push_back(parse_message(message));
    }

    return results;
}

std::string render_results_as_json(const std::vector<ParseResult>& results) {
    std::ostringstream stream;
    stream << "{\n";
    append_indent(stream, 1);
    stream << "\"parsed\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const ParseResult& result = results[i];
        append_indent(stream, 2);
        stream << "{\n";
        if (result.matched) {
            append_indent(stream, 3);
            stream << "\"rule\": {\n";
            append_indent(stream, 4);
            stream << "\"id\": \"" << escape_json_string(result.message_rule_id) << "\",\n";
            append_indent(stream, 4);
            stream << "\"name\": \"" << escape_json_string(result.message_rule_name) << "\",\n";
            append_indent(stream, 4);
            stream << "\"token_extraction\": [";
            for (std::size_t idx = 0; idx < result.token_extraction.size(); ++idx) {
                if (idx != 0) {
                    stream << ", ";
                }
                stream << "\"" << escape_json_string(result.token_extraction[idx]) << "\"";
            }
            stream << "]\n";
            append_indent(stream, 3);
            stream << "},\n";

            append_indent(stream, 3);
            stream << "\"event\": {\n";
            append_indent(stream, 4);
            stream << "\"name\": \"" << escape_json_string(result.event_name) << "\",\n";
            append_indent(stream, 4);
            stream << "\"pattern_id\": \"" << escape_json_string(result.event_pattern_id) << "\"\n";
            append_indent(stream, 3);
            stream << "},\n";

            append_indent(stream, 3);
            stream << "\"tokens\": {\n";
            const RenderNode token_tree = build_render_tree(result);
            render_object_children(stream, token_tree, 4);
            append_indent(stream, 3);
            stream << "}";

            if (!result.properties.empty()) {
                stream << ",\n";
                append_indent(stream, 3);
                stream << "\"property\": {\n";
                RenderNode property_tree;
                for (const auto& property : result.properties) {
                    insert_render_token(property_tree, property.first, property.second);
                }
                render_object_children(stream, property_tree, 4);
                append_indent(stream, 3);
                stream << "}";
            }
        } else {
            append_indent(stream, 3);
            stream << "\"rule\": null";
        }

        if (!result.errors.empty()) {
            stream << ",\n";
            append_indent(stream, 3);
            stream << "\"errors\": [\n";
            for (std::size_t idx = 0; idx < result.errors.size(); ++idx) {
                if (idx != 0) {
                    stream << ",\n";
                }
                append_indent(stream, 4);
                stream << "\"" << escape_json_string(result.errors[idx]) << "\"";
            }
            stream << "\n";
            append_indent(stream, 3);
            stream << "]";
        }

        stream << "\n";
        append_indent(stream, 2);
        stream << "}";
        if (i + 1 != results.size()) {
            stream << ",";
        }
        stream << "\n";
    }

    append_indent(stream, 1);
    stream << "]\n";
    stream << "}";
    return stream.str();
}

std::string render_result_as_json(const ParseResult& result) {
    return render_results_as_json(std::vector<ParseResult>{result});
}

} // namespace parser_framework
