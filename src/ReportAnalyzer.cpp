#include "parser_framework/ReportAnalyzer.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

#include "parser_framework/DynamicPropertyEngine.hpp"

namespace parser_framework {

namespace {

std::string normalize_report_property_name(const std::string& name) {
    static const std::string prefix = "report.";
    if (name.rfind(prefix, 0) == 0) {
        return name.substr(prefix.size());
    }

    return name;
}

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

void remember_child_order(RenderNode& node, const std::string& child_name) {
    if (node.children.find(child_name) == node.children.end()) {
        node.child_order.push_back(child_name);
    }
}

void insert_value(RenderNode& root, const std::string& name, const std::string& value) {
    std::size_t start = 0;
    RenderNode* current = &root;

    while (start != std::string::npos) {
        const std::size_t dot = name.find('.', start);
        const std::string segment = name.substr(start,
            dot == std::string::npos ? std::string::npos : dot - start);
        remember_child_order(*current, segment);
        std::unique_ptr<RenderNode>& child = current->children[segment];
        if (!child) {
            child = std::make_unique<RenderNode>();
        }
        current = child.get();
        if (dot == std::string::npos) {
            current->value = value;
            return;
        }
        start = dot + 1;
    }
}

void render_node_as_json(std::ostringstream& stream, const RenderNode& node, std::size_t depth);

void render_children(std::ostringstream& stream, const RenderNode& node, std::size_t depth) {
    bool first = true;

    if (node.value.has_value()) {
        append_indent(stream, depth);
        stream << "\"raw\": \"" << escape_json_string(*node.value) << "\"";
        first = false;
    }

    for (const auto& child_name : node.child_order) {
        auto it = node.children.find(child_name);
        if (it == node.children.end()) {
            continue;
        }

        if (!first) {
            stream << ",\n";
        }
        append_indent(stream, depth);
        stream << "\"" << escape_json_string(child_name) << "\": ";
        render_node_as_json(stream, *it->second, depth);
        first = false;
    }

    if (!first) {
        stream << "\n";
    }
}

void render_node_as_json(std::ostringstream& stream, const RenderNode& node, std::size_t depth) {
    if (node.children.empty()) {
        stream << "\"" << escape_json_string(node.value.value_or(std::string{})) << "\"";
        return;
    }

    stream << "{\n";
    render_children(stream, node, depth + 1);
    append_indent(stream, depth);
    stream << "}";
}

RenderNode build_tree(const std::map<std::string, std::string>& values) {
    RenderNode root;
    for (const auto& value : values) {
        insert_value(root, value.first, value.second);
    }

    return root;
}

template <typename Handler>
void for_each_yaml_file(const std::filesystem::path& input, Handler&& handler) {
    namespace fs = std::filesystem;

    if (fs::is_regular_file(input)) {
        handler(input);
        return;
    }

    if (!fs::is_directory(input)) {
        throw std::runtime_error("Report rules path is not a file or directory [" + input.string() + "]");
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

void load_report_file(const std::filesystem::path& file, std::vector<ReportRule>& rules) {
    YAML::Node root = YAML::LoadFile(file.string());
    const YAML::Node reports = root["reports"];
    if (!reports || !reports.IsSequence()) {
        return;
    }

    for (const auto& report_node : reports) {
        ReportRule rule;
        rule.id = report_node["id"].as<std::string>();
        rule.name = report_node["name"].as<std::string>();

        const YAML::Node bindings = report_node["bindings"];
        if (bindings && bindings.IsSequence()) {
            for (const auto& binding : bindings) {
                rule.bindings.push_back(binding.as<std::string>());
            }
        }

        const YAML::Node dynamic_properties = report_node["dynamic_properties"];
        if (dynamic_properties && dynamic_properties.IsSequence()) {
            for (const auto& expression : dynamic_properties) {
                rule.dynamic_properties.push_back(expression.as<std::string>());
            }
        }

        rules.push_back(std::move(rule));
    }
}

} // namespace

ReportAnalyzer::ReportAnalyzer(std::vector<ReportRule> rules)
    : m_rules(std::move(rules)) {}

void ReportAnalyzer::add_rule(ReportRule rule) {
    m_rules.push_back(std::move(rule));
}

std::vector<ReportFinding> ReportAnalyzer::analyze(const ParseResult& result) const {
    std::vector<ReportFinding> findings;
    const DynamicPropertyEngine engine;

    for (const auto& rule : m_rules) {
        if (!rule.bindings.empty() &&
            std::find(rule.bindings.begin(), rule.bindings.end(), result.message_rule_id) == rule.bindings.end()) {
            continue;
        }

        std::vector<std::string> errors;
        const std::map<std::string, std::string> evaluated =
            engine.evaluate(rule.dynamic_properties, result, errors);
        if (!errors.empty()) {
            continue;
        }

        std::map<std::string, std::string> properties;
        for (const auto& property : evaluated) {
            properties[normalize_report_property_name(property.first)] = property.second;
        }

        if (properties.empty()) {
            continue;
        }

        findings.push_back(ReportFinding{
            rule.id,
            rule.name,
            result.message_rule_id,
            result.message_rule_name,
            result.event_name,
            std::move(properties)
        });
    }

    return findings;
}

std::vector<ReportFinding> ReportAnalyzer::analyze(const std::vector<ParseResult>& results) const {
    std::vector<ReportFinding> findings;

    for (const auto& result : results) {
        const std::vector<ReportFinding> next = analyze(result);
        findings.insert(findings.end(), next.begin(), next.end());
    }

    return findings;
}

namespace ReportRuleLoader {

std::vector<ReportRule> load_rules(const std::string& path) {
    namespace fs = std::filesystem;

    const fs::path input(path);
    if (!fs::exists(input)) {
        throw std::runtime_error("Report rules path does not exist [" + path + "]");
    }

    std::vector<ReportRule> rules;
    for_each_yaml_file(input, [&](const fs::path& file) {
        load_report_file(file, rules);
    });

    return rules;
}

} // namespace ReportRuleLoader

std::string render_reports_as_json(const std::vector<ReportFinding>& reports) {
    std::ostringstream stream;
    stream << "{\n";
    append_indent(stream, 1);
    stream << "\"reports\": [\n";

    for (std::size_t i = 0; i < reports.size(); ++i) {
        const ReportFinding& report = reports[i];
        append_indent(stream, 2);
        stream << "{\n";

        append_indent(stream, 3);
        stream << "\"rule\": {\n";
        append_indent(stream, 4);
        stream << "\"id\": \"" << escape_json_string(report.id) << "\",\n";
        append_indent(stream, 4);
        stream << "\"name\": \"" << escape_json_string(report.name) << "\"\n";
        append_indent(stream, 3);
        stream << "},\n";

        append_indent(stream, 3);
        stream << "\"source\": {\n";
        append_indent(stream, 4);
        stream << "\"message_rule_id\": \"" << escape_json_string(report.message_rule_id) << "\",\n";
        append_indent(stream, 4);
        stream << "\"message_rule_name\": \"" << escape_json_string(report.message_rule_name) << "\",\n";
        append_indent(stream, 4);
        stream << "\"event_name\": \"" << escape_json_string(report.event_name) << "\"\n";
        append_indent(stream, 3);
        stream << "},\n";

        append_indent(stream, 3);
        stream << "\"report\": {\n";
        const RenderNode tree = build_tree(report.properties);
        render_children(stream, tree, 4);
        append_indent(stream, 3);
        stream << "}\n";

        append_indent(stream, 2);
        stream << "}";
        if (i + 1 != reports.size()) {
            stream << ",";
        }
        stream << "\n";
    }

    append_indent(stream, 1);
    stream << "]\n";
    stream << "}";
    return stream.str();
}

} // namespace parser_framework
