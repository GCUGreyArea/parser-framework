#include "parser_framework/ReportAnalyzer.hpp"

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <set>
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

void load_correlation_file(const std::filesystem::path& file, std::vector<CorrelationRule>& rules) {
    YAML::Node root = YAML::LoadFile(file.string());
    const YAML::Node correlations = root["correlations"];
    if (!correlations || !correlations.IsSequence()) {
        return;
    }

    for (const auto& correlation_node : correlations) {
        CorrelationRule rule;
        rule.id = correlation_node["id"].as<std::string>();
        rule.name = correlation_node["name"].as<std::string>();
        rule.family = correlation_node["family"].as<std::string>();
        if (const YAML::Node min_distinct_systems = correlation_node["min_distinct_systems"]) {
            rule.min_distinct_systems = min_distinct_systems.as<std::size_t>();
        }

        const YAML::Node systems = correlation_node["systems"];
        if (systems && systems.IsSequence()) {
            for (const auto& system : systems) {
                rule.systems.push_back(system.as<std::string>());
            }
        }

        rules.push_back(std::move(rule));
    }
}

std::string join_values(const std::vector<std::string>& values) {
    std::ostringstream stream;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            stream << ", ";
        }
        stream << values[i];
    }

    return stream.str();
}

std::vector<ReportFinding> correlate_findings(
    const std::vector<ReportFinding>& findings,
    const std::vector<CorrelationRule>& rules) {
    std::vector<ReportFinding> correlated;
    if (rules.empty()) {
        return correlated;
    }

    struct Group {
        std::string source_ip;
        std::string family;
        std::vector<const ReportFinding*> findings;
        std::vector<std::string> systems_in_order;
        std::set<std::string> systems;
    };

    std::map<std::pair<std::string, std::string>, Group> groups;
    for (const auto& finding : findings) {
        const auto source_ip_it = finding.properties.find("source.ip");
        const auto family_it = finding.properties.find("attack.family");
        const auto system_it = finding.properties.find("system");
        if (source_ip_it == finding.properties.end() ||
            family_it == finding.properties.end() ||
            system_it == finding.properties.end()) {
            continue;
        }

        const std::pair<std::string, std::string> key(source_ip_it->second, family_it->second);
        Group& group = groups[key];
        group.source_ip = source_ip_it->second;
        group.family = family_it->second;
        group.findings.push_back(&finding);
        if (group.systems.insert(system_it->second).second) {
            group.systems_in_order.push_back(system_it->second);
        }
    }

    for (const auto& rule : rules) {
        for (const auto& entry : groups) {
            const Group& group = entry.second;
            if (group.family != rule.family) {
                continue;
            }
            if (group.systems.size() < rule.min_distinct_systems) {
                continue;
            }

            bool required_systems_present = true;
            for (const auto& required : rule.systems) {
                if (group.systems.find(required) == group.systems.end()) {
                    required_systems_present = false;
                    break;
                }
            }
            if (!required_systems_present) {
                continue;
            }

            ReportFinding correlated_finding;
            correlated_finding.id = rule.id;
            correlated_finding.name = rule.name;

            const ReportFinding& first = *group.findings.front();
            correlated_finding.message_rule_id = first.message_rule_id;
            correlated_finding.message_rule_name = first.message_rule_name;
            correlated_finding.event_name = first.event_name;

            std::set<std::string> seen_sources;
            for (const ReportFinding* finding : group.findings) {
                for (const auto& source : finding->sources) {
                    const std::string key =
                        source.message_rule_id + "\n" + source.message_rule_name + "\n" + source.event_name;
                    if (seen_sources.insert(key).second) {
                        correlated_finding.sources.push_back(source);
                    }
                }
            }

            correlated_finding.properties["event"] = "multi_system_breach_attempt";
            correlated_finding.properties["phase"] =
                first.properties.count("phase") ? first.properties.at("phase") : "initial_access";
            correlated_finding.properties["system"] = "multi_system";
            correlated_finding.properties["source.ip"] = group.source_ip;
            correlated_finding.properties["attack.family"] = group.family;
            if (first.properties.count("attack.name") != 0) {
                correlated_finding.properties["attack.name"] = first.properties.at("attack.name");
            }
            if (first.properties.count("technique.id") != 0) {
                correlated_finding.properties["technique.id"] = first.properties.at("technique.id");
            }
            if (first.properties.count("tactic.id") != 0) {
                correlated_finding.properties["tactic.id"] = first.properties.at("tactic.id");
            }
            correlated_finding.properties["systems.count"] = std::to_string(group.systems.size());
            correlated_finding.properties["systems.names"] = join_values(group.systems_in_order);
            correlated_finding.properties["evidence.count"] = std::to_string(group.findings.size());
            correlated_finding.properties["summary"] =
                "Observed " + group.family + " activity from " + group.source_ip +
                " across " + join_values(group.systems_in_order);
            correlated_finding.properties["outcome"] = "observed";

            correlated.push_back(std::move(correlated_finding));
        }
    }

    return correlated;
}

void render_source(std::ostringstream& stream, const ReportSource& source, std::size_t depth) {
    append_indent(stream, depth);
    stream << "{\n";
    append_indent(stream, depth + 1);
    stream << "\"message_rule_id\": \"" << escape_json_string(source.message_rule_id) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"message_rule_name\": \"" << escape_json_string(source.message_rule_name) << "\",\n";
    append_indent(stream, depth + 1);
    stream << "\"event_name\": \"" << escape_json_string(source.event_name) << "\"\n";
    append_indent(stream, depth);
    stream << "}";
}

} // namespace

ReportAnalyzer::ReportAnalyzer(std::vector<ReportRule> rules)
    : m_rules(std::move(rules)) {}

ReportAnalyzer::ReportAnalyzer(ReportRuleSet ruleset)
    : m_rules(std::move(ruleset.reports)),
      m_correlation_rules(std::move(ruleset.correlations)) {}

void ReportAnalyzer::add_rule(ReportRule rule) {
    m_rules.push_back(std::move(rule));
}

void ReportAnalyzer::add_correlation_rule(CorrelationRule rule) {
    m_correlation_rules.push_back(std::move(rule));
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
            {ReportSource{result.message_rule_id, result.message_rule_name, result.event_name}},
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

    const std::vector<ReportFinding> correlated = correlate_findings(findings, m_correlation_rules);
    findings.insert(findings.end(), correlated.begin(), correlated.end());

    return findings;
}

namespace ReportRuleLoader {

ReportRuleSet load_rules(const std::string& path) {
    namespace fs = std::filesystem;

    const fs::path input(path);
    if (!fs::exists(input)) {
        throw std::runtime_error("Report rules path does not exist [" + path + "]");
    }

    ReportRuleSet ruleset;
    for_each_yaml_file(input, [&](const fs::path& file) {
        load_report_file(file, ruleset.reports);
        load_correlation_file(file, ruleset.correlations);
    });

    return ruleset;
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
        const ReportSource primary_source{
            report.message_rule_id,
            report.message_rule_name,
            report.event_name
        };
        append_indent(stream, 4);
        stream << "\"message_rule_id\": \"" << escape_json_string(primary_source.message_rule_id) << "\",\n";
        append_indent(stream, 4);
        stream << "\"message_rule_name\": \"" << escape_json_string(primary_source.message_rule_name) << "\",\n";
        append_indent(stream, 4);
        stream << "\"event_name\": \"" << escape_json_string(primary_source.event_name) << "\"\n";
        append_indent(stream, 3);
        stream << "}";
        if (report.sources.size() > 1) {
            stream << ",\n";
            append_indent(stream, 3);
            stream << "\"sources\": [\n";
            for (std::size_t source_index = 0; source_index < report.sources.size(); ++source_index) {
                render_source(stream, report.sources[source_index], 4);
                if (source_index + 1 != report.sources.size()) {
                    stream << ",";
                }
                stream << "\n";
            }
            append_indent(stream, 3);
            stream << "]";
        }
        stream << ",\n";

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
