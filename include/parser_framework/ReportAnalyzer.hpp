#ifndef PARSER_FRAMEWORK_REPORT_ANALYZER_HPP
#define PARSER_FRAMEWORK_REPORT_ANALYZER_HPP

#include <map>
#include <string>
#include <vector>

#include "parser_framework/ParserFramework.hpp"

namespace parser_framework {

struct ReportRule {
    std::string id;
    std::string name;
    std::vector<std::string> bindings;
    std::vector<std::string> dynamic_properties;
};

struct ReportFinding {
    std::string id;
    std::string name;
    std::string message_rule_id;
    std::string message_rule_name;
    std::string event_name;
    std::map<std::string, std::string> properties;
};

class ReportAnalyzer {
public:
    ReportAnalyzer() = default;
    explicit ReportAnalyzer(std::vector<ReportRule> rules);

    void add_rule(ReportRule rule);
    std::vector<ReportFinding> analyze(const ParseResult& result) const;
    std::vector<ReportFinding> analyze(const std::vector<ParseResult>& results) const;

private:
    std::vector<ReportRule> m_rules;
};

namespace ReportRuleLoader {

std::vector<ReportRule> load_rules(const std::string& path);

} // namespace ReportRuleLoader

std::string render_reports_as_json(const std::vector<ReportFinding>& reports);

} // namespace parser_framework

#endif
