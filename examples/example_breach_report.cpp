#include <iostream>
#include <string>
#include <vector>

#include "parser_framework/MessageLoader.hpp"
#include "parser_framework/ParserFramework.hpp"
#include "parser_framework/ReportAnalyzer.hpp"
#include "parser_framework/RuleLoader.hpp"

using namespace parser_framework;

extern "C" const char* __asan_default_options() {
    return "detect_leaks=0";
}

extern "C" const char* __lsan_default_options() {
    return "detect_leaks=0";
}

namespace {

std::vector<std::string> read_messages(int argc, char** argv, int first_message_arg) {
    std::vector<std::string> paths;
    for (int i = first_message_arg; i < argc; ++i) {
        paths.push_back(argv[i]);
    }

    if (paths.empty()) {
        paths.push_back("messages");
    }

    return MessageLoader::load_messages_from_paths(paths);
}

} // namespace

int main(int argc, char** argv) {
    try {
        const std::string rules_path = argc > 1 ? argv[1] : "rules";
        const std::string report_rules_path = argc > 2 ? argv[2] : "report_rules";

        ParserFramework framework(RuleLoader::load_rules(rules_path));
        ReportAnalyzer analyzer(ReportRuleLoader::load_rules(report_rules_path));
        const std::vector<std::string> messages = read_messages(argc, argv, 3);

        if (messages.empty()) {
            std::cerr << "No messages found to analyze\n";
            return 1;
        }

        const std::vector<ParseResult> results = framework.parse_messages(messages);
        const std::vector<ReportFinding> reports = analyzer.analyze(results);
        std::cout << render_reports_as_json(reports) << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }
}
