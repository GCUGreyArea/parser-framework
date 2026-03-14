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
        return {
            "CheckPoint 9929 - [action:\"Accept\"; conn_direction:\"Outgoing\"; contextnum:\"5\"; flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; sequencenum:\"110\"; time:\"1630417161\"; version:\"5\"; __policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; match_id:\"48\"; rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; proto:\"6\"; s_port:\"44853\"; service:\"443\"; service_id:\"https\";]"
        };
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
    const std::vector<std::string> messages = read_messages(argc, argv, 2);
    const std::vector<ParseResult> results = framework.parse_messages(messages);

    std::cout << render_results_as_json(results) << "\n";
    return 0;
}
