#ifndef PARSER_FRAMEWORK_REPORT_ANALYZER_HPP
#define PARSER_FRAMEWORK_REPORT_ANALYZER_HPP

#include <map>
#include <string>
#include <vector>

#include "parser_framework/ParserFramework.hpp"

namespace parser_framework {

struct ReportSource {
    std::string message_rule_id;
    std::string message_rule_name;
    std::string event_name;
};

struct ReportRule {
    std::string id;
    std::string name;
    std::vector<std::string> bindings;
    std::vector<std::string> dynamic_properties;
};

struct CorrelationRule {
    std::string id;
    std::string name;
    std::string family;
    std::size_t min_distinct_systems {2};
    std::vector<std::string> systems;
};

struct ReportRuleSet {
    std::vector<ReportRule> reports;
    std::vector<CorrelationRule> correlations;
};

struct ReportFinding {
    std::string id;
    std::string name;
    std::string message_rule_id;
    std::string message_rule_name;
    std::string event_name;
    std::vector<ReportSource> sources;
    std::map<std::string, std::string> properties;
};

class ReportAnalyzer {
public:
    ReportAnalyzer() = default;
    explicit ReportAnalyzer(std::vector<ReportRule> rules);
    explicit ReportAnalyzer(ReportRuleSet ruleset);

    void add_rule(ReportRule rule);
    void add_correlation_rule(CorrelationRule rule);
    std::vector<ReportFinding> analyze(const ParseResult& result) const;
    std::vector<ReportFinding> analyze(const std::vector<ParseResult>& results) const;

private:
    std::vector<ReportRule> m_rules;
    std::vector<CorrelationRule> m_correlation_rules;
};

namespace ReportRuleLoader {

ReportRuleSet load_rules(const std::string& path);

} // namespace ReportRuleLoader

std::string render_reports_as_json(const std::vector<ReportFinding>& reports);

} // namespace parser_framework

#endif
