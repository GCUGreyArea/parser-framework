#include <string>

#include <gtest/gtest.h>

#include "parser_framework/ParserFramework.hpp"
#include "parser_framework/RuleLoader.hpp"

using namespace parser_framework;

namespace {

const Token* find_token(const ParseResult& result, const std::string& name) {
    for (const auto& token : result.tokens) {
        if (token.name == name) {
            return &token;
        }
    }

    return nullptr;
}

std::string checkpoint_message() {
    return "CheckPoint 9929 - [action:\"Accept\"; conn_direction:\"Outgoing\"; contextnum:\"5\"; "
           "flags:\"7263232\"; ifdir:\"outbound\"; ifname:\"eth2\"; "
           "loguid:\"{0x35904e9,0x4655026c,0xeafacc55,0x657c933c}\"; sequencenum:\"110\"; "
           "time:\"1630417161\"; version:\"5\"; "
           "__policy_id_tag:\"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]\"; "
           "layer_uuid:\"9d7748a0-845f-491d-9eef-1fb41680bc35\"; match_id:\"48\"; "
           "rule_uid:\"8bf81033-b6c7-44fe-a88a-2068c155f50e\"; proto:\"6\"; s_port:\"44853\"; "
           "service:\"443\"; service_id:\"https\";]";
}

} // namespace

TEST(ParserFramework, ParsesCapturedKvSectionAndRendersJsonOutput) {
    MessageRule rule;
    rule.id = "checkpoint-id";
    rule.name = "CheckPoint";
    rule.filters = {
        MessageFilterRule{
            "^CheckPoint [0-9]+ - \\[(.*)\\]$",
            {NamedCapture{"fields", 1}}
        }
    };

    SectionRule section;
    section.id = "fields";
    section.kind = SectionKind::KV;
    section.extract_name = "fields";
    section.ruleset_name = "CheckPointKV";
    section.declared_tokens = {
        {"action", "string"},
        {"contextnum", "int"},
        {"service_id", "string"}
    };

    rule.sections = {section};

    ParserFramework framework({rule});
    ParseResult result = framework.parse_message(checkpoint_message());

    ASSERT_TRUE(result.matched);
    ASSERT_TRUE(result.errors.empty());

    const Token* action = find_token(result, "action");
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->value, "Accept");

    const Token* contextnum = find_token(result, "contextnum");
    ASSERT_NE(contextnum, nullptr);
    EXPECT_EQ(contextnum->value, "5");

    const Token* service_id = find_token(result, "service_id");
    ASSERT_NE(service_id, nullptr);
    EXPECT_EQ(service_id->value, "https");

    const std::string json = render_result_as_json(result);
    EXPECT_NE(json.find("\"rule\":{\"id\":\"checkpoint-id\",\"name\":\"CheckPoint\"}"), std::string::npos);
    EXPECT_NE(json.find("\"action\":\"Accept\""), std::string::npos);
    EXPECT_NE(json.find("\"service_id\":\"https\""), std::string::npos);
}

TEST(RuleLoader, LoadsCheckPointRulesAndParsesMessage) {
    const std::vector<MessageRule> rules = RuleLoader::load_rules("rules");
    ASSERT_EQ(rules.size(), 1u);

    ParserFramework framework(rules);
    ParseResult result = framework.parse_message(checkpoint_message());

    ASSERT_TRUE(result.matched);
    ASSERT_TRUE(result.errors.empty());
    EXPECT_EQ(result.message_rule_id, "46432110-a2a1-4bc9-8e78-fe30591375e1");

    const Token* action = find_token(result, "action");
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->value, "Accept");

    const Token* proto = find_token(result, "proto");
    ASSERT_NE(proto, nullptr);
    EXPECT_EQ(proto->value, "6");

    const Token* service = find_token(result, "service");
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->value, "443");

    const Token* service_id = find_token(result, "service_id");
    ASSERT_NE(service_id, nullptr);
    EXPECT_EQ(service_id->value, "https");
}
