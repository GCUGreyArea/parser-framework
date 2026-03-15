#include <iostream>
#include <string>
#include <vector>

#include "parser_framework/MessageLoader.hpp"
#include "parser_framework/ParserFramework.hpp"
#include "parser_framework/RuleLoader.hpp"
#include "parser_framework/utils/Args.hpp"

using namespace parser_framework;

extern "C" const char* __asan_default_options() {
    return "detect_leaks=0";
}

extern "C" const char* __lsan_default_options() {
    return "detect_leaks=0";
}

namespace {

void print_usage() {
    std::cout << "Usage: example_parser [-r|--rules <path>] [-m|--messages <path> [<path>...]]\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        utils::Args args(argc, argv);
        args.add_string_value("-r", "--rules", "rules");
        args.add_string_values("-m", "--messages", {"messages"});
        args.add_key("-h", "--help");

        if (args.is_key_present("-h")) {
            print_usage();
            return 0;
        }

        const std::string rules_path = args.get_string_value("-r");
        ParserFramework framework(RuleLoader::load_rules(rules_path));
        const std::vector<std::string> messages = MessageLoader::load_messages_from_paths(args.get_string_values("-m"));

        if (messages.empty()) {
            std::cerr << "No messages found to parse\n";
            return 1;
        }

        const std::vector<ParseResult> results = framework.parse_messages(messages);
        std::cout << render_results_as_json(results) << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }
}
