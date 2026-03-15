#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "parser_framework/ParserFramework.hpp"
#include "parser_framework/RuleLoader.hpp"

using namespace parser_framework;

namespace {

std::vector<std::string> read_messages(int argc, char** argv, int first_message_arg) {
    if (argc <= first_message_arg) {
        return {};
    }

    std::vector<std::string> messages;
    for (int i = first_message_arg; i < argc; ++i) {
        std::ifstream input(argv[i]);
        if (!input.is_open()) {
            std::cerr << "Failed to open input file [" << argv[i] << "]\n";
            continue;
        }

        for (std::string line; std::getline(input, line); ) {
            if (!line.empty()) {
                messages.push_back(line);
            }
        }
    }

    return messages;
}

} // namespace

int main(int argc, char** argv) {
    const std::string rules_path = argc > 1 ? argv[1] : "rules";
    ParserFramework framework(RuleLoader::load_rules(rules_path));
    std::vector<std::string> messages = read_messages(argc, argv, 2);
    if (messages.empty()) {
        messages = RuleLoader::load_example_messages(rules_path);
    }

    if (messages.empty()) {
        std::cerr << "No example messages found in rules path [" << rules_path << "]\n";
        return 1;
    }

    const std::vector<ParseResult> results = framework.parse_messages(messages);

    std::cout << render_results_as_json(results) << "\n";
    return 0;
}
