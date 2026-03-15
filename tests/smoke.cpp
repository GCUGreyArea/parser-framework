#include <algorithm>
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

std::string fortigate_message() {
    return "date=2017-11-15 time=11:44:16 logid=\"0000000013\" type=\"traffic\" subtype=\"forward\" "
           "level=\"notice\" vd=\"vdom1\" eventtime=1510775056 srcip=10.1.100.155 srcname=\"pc1\" "
           "srcport=40772 srcintf=\"port12\" srcintfrole=\"undefined\" dstip=35.197.51.42 "
           "dstname=\"fortiguard.com\" dstport=443 dstintf=\"port11\" dstintfrole=\"undefined\" "
           "poluuid=\"707a0d88-cf9e-51e7-bbc7-4d421660557b\" sessionid=22921 proto=6 action=\"close\" "
           "policyid=1 policytype=\"policy\" service=\"HTTPS\" dstcountry=\"United States\" "
           "srccountry=\"Reserved\" trandisp=\"snat\" transip=172.16.200.2 transport=40772 "
           "appid=40568 app=\"HTTPS.BROWSER\" appcat=\"Web.Client\" apprisk=\"medium\" duration=2 "
           "sentbyte=1850 rcvdbyte=39898 sentpkt=25 rcvdpkt=37 utmaction=\"allow\" countapp=1";
}

std::string aws_waf_message() {
    return "{\"timestamp\":1576280412771,\"formatVersion\":1,"
           "\"webaclId\":\"arn:aws:wafv2:ap-southeast-2:111122223333:regional/webacl/STMTest/"
           "1EXAMPLE-2ARN-3ARN-4ARN-123456EXAMPLE\",\"terminatingRuleId\":\"STMTest_SQLi_XSS\","
           "\"terminatingRuleType\":\"REGULAR\",\"action\":\"BLOCK\","
           "\"terminatingRuleMatchDetails\":[{\"conditionType\":\"SQL_INJECTION\","
           "\"sensitivityLevel\":\"HIGH\",\"location\":\"HEADER\",\"matchedData\":[\"10\",\"AND\",\"1\"]}],"
           "\"httpSourceName\":\"-\",\"httpSourceId\":\"-\",\"ruleGroupList\":[],"
           "\"rateBasedRuleList\":[],\"nonTerminatingMatchingRules\":[],\"httpRequest\":{"
           "\"clientIp\":\"1.1.1.1\",\"country\":\"AU\",\"headers\":[{\"name\":\"Host\","
           "\"value\":\"localhost:1989\"},{\"name\":\"User-Agent\",\"value\":\"curl/7.61.1\"},"
           "{\"name\":\"Accept\",\"value\":\"*/*\"},{\"name\":\"x-stm-test\",\"value\":\"10 AND 1=1\"}],"
           "\"uri\":\"/\",\"args\":\"\",\"httpVersion\":\"HTTP/1.1\",\"httpMethod\":\"POST\","
           "\"requestId\":\"aws-waf-request-001\"},\"labels\":[{\"name\":\"awswaf:managed:token:absent\"}]}";
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
    section.ruleset_id = "checkpoint-kv";
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
    EXPECT_EQ(result.token_extraction.size(), 2u);
    EXPECT_EQ(result.token_extraction[0], "checkpoint-id");
    EXPECT_EQ(result.token_extraction[1], "checkpoint-kv");

    const std::string json = render_result_as_json(result);
    EXPECT_NE(json.find("\"id\": \"checkpoint-id\""), std::string::npos);
    EXPECT_NE(json.find("\"token_extraction\": [\"checkpoint-id\", \"checkpoint-kv\"]"), std::string::npos);
    EXPECT_NE(json.find("\"action\": \"Accept\""), std::string::npos);
    EXPECT_NE(json.find("\"service_id\": \"https\""), std::string::npos);
}

TEST(ParserFramework, RendersNestedTokenObjectsForDottedNames) {
    ParseResult result;
    result.matched = true;
    result.message_rule_id = "checkpoint-id";
    result.message_rule_name = "CheckPoint";
    result.event_name = "CheckPoint";
    result.event_pattern_id = "checkpoint-id";
    result.token_extraction = {"checkpoint-id", "checkpoint-kv", "checkpoint-policy-tag"};
    result.tokens = {
        {"__policy_id_tag", "product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\]", "fields", "kv", "__policy_id_tag", 0, 0},
        {"__policy_id_tag.product", "VPN-1 & FireWall-1", "policy", "regex", {}, 0, 0},
        {"__policy_id_tag.db_tag", "599255D7-8B94-704F-9D9A-CFA3719EA5CE", "policy", "regex", {}, 0, 0},
        {"__policy_id_tag.mgmt", "bos1cpmgmt01", "policy", "regex", {}, 0, 0},
        {"__policy_id_tag.date", "1630333752", "policy", "regex", {}, 0, 0},
        {"__policy_id_tag.policy_name", "<policy>", "policy", "regex", {}, 0, 0}
    };

    const std::string json = render_result_as_json(result);
    EXPECT_NE(json.find("\"__policy_id_tag\": {\n"), std::string::npos);
    EXPECT_NE(json.find("\"raw\": \"product=VPN-1 & FireWall-1[db_tag={599255D7-8B94-704F-9D9A-CFA3719EA5CE};mgmt=bos1cpmgmt01;date=1630333752;policy_name=<policy>\\\\]\""), std::string::npos);
    EXPECT_NE(json.find("\"product\": \"VPN-1 & FireWall-1\""), std::string::npos);
    EXPECT_NE(json.find("\"db_tag\": \"599255D7-8B94-704F-9D9A-CFA3719EA5CE\""), std::string::npos);
    EXPECT_NE(json.find("\"mgmt\": \"bos1cpmgmt01\""), std::string::npos);
    EXPECT_NE(json.find("\"date\": \"1630333752\""), std::string::npos);
    EXPECT_NE(json.find("\"policy_name\": \"<policy>\""), std::string::npos);
    EXPECT_EQ(json.find("\"__policy_id_tag.product\""), std::string::npos);
}

TEST(RuleLoader, LoadsCheckPointRulesAndParsesMessage) {
    const std::vector<MessageRule> rules = RuleLoader::load_rules("rules");
    ASSERT_EQ(rules.size(), 3u);

    ParserFramework framework(rules);
    ParseResult result = framework.parse_message(checkpoint_message());

    ASSERT_TRUE(result.matched);
    ASSERT_TRUE(result.errors.empty());
    EXPECT_EQ(result.message_rule_id, "46432110-a2a1-4bc9-8e78-fe30591375e1");
    ASSERT_EQ(result.token_extraction.size(), 3u);
    EXPECT_EQ(result.token_extraction[0], "46432110-a2a1-4bc9-8e78-fe30591375e1");
    EXPECT_EQ(result.token_extraction[1], "9b1c8e5c-7a0b-4f1e-9c3a-2f8e5d6c7b8a");
    EXPECT_EQ(result.token_extraction[2], "7c55684a-8746-43d8-a34a-130ebd3ec0d4");

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

    const Token* policy_product = find_token(result, "__policy_id_tag.product");
    ASSERT_NE(policy_product, nullptr);
    EXPECT_EQ(policy_product->value, "VPN-1 & FireWall-1");

    const Token* policy_db_tag = find_token(result, "__policy_id_tag.db_tag");
    ASSERT_NE(policy_db_tag, nullptr);
    EXPECT_EQ(policy_db_tag->value, "599255D7-8B94-704F-9D9A-CFA3719EA5CE");

    const Token* policy_mgmt = find_token(result, "__policy_id_tag.mgmt");
    ASSERT_NE(policy_mgmt, nullptr);
    EXPECT_EQ(policy_mgmt->value, "bos1cpmgmt01");

    const Token* policy_name = find_token(result, "__policy_id_tag.policy_name");
    ASSERT_NE(policy_name, nullptr);
    EXPECT_EQ(policy_name->value, "<policy>");
}

TEST(RuleLoader, LoadsFortiGateRulesAndParsesMessage) {
    ParserFramework framework(RuleLoader::load_rules("rules"));
    ParseResult result = framework.parse_message(fortigate_message());

    ASSERT_TRUE(result.matched);
    ASSERT_TRUE(result.errors.empty());
    EXPECT_EQ(result.message_rule_name, "FortiGate Traffic");
    ASSERT_EQ(result.token_extraction.size(), 2u);
    EXPECT_EQ(result.token_extraction[0], "5ceff8f3-1a41-4ed3-a292-c15e929e4c5d");
    EXPECT_EQ(result.token_extraction[1], "3cd579a8-fb20-4c59-84d8-2456d8d55f59");

    const Token* srcip = find_token(result, "srcip");
    ASSERT_NE(srcip, nullptr);
    EXPECT_EQ(srcip->value, "10.1.100.155");

    const Token* dstip = find_token(result, "dstip");
    ASSERT_NE(dstip, nullptr);
    EXPECT_EQ(dstip->value, "35.197.51.42");

    const Token* service = find_token(result, "service");
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->value, "HTTPS");

    const Token* utmaction = find_token(result, "utmaction");
    ASSERT_NE(utmaction, nullptr);
    EXPECT_EQ(utmaction->value, "allow");
}

TEST(RuleLoader, LoadsAwsWafRulesAndParsesMessage) {
    ParserFramework framework(RuleLoader::load_rules("rules"));
    ParseResult result = framework.parse_message(aws_waf_message());

    ASSERT_TRUE(result.matched);
    ASSERT_TRUE(result.errors.empty());
    EXPECT_EQ(result.message_rule_name, "AWS WAF");
    ASSERT_EQ(result.token_extraction.size(), 2u);
    EXPECT_EQ(result.token_extraction[0], "1c507cb9-91d4-4024-871d-49f38ee08f36");
    EXPECT_EQ(result.token_extraction[1], "4f8db6bf-6934-4d04-85bf-d4c431ebc0f6");

    const Token* action = find_token(result, "action");
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->value, "BLOCK");

    const Token* client_ip = find_token(result, "client_ip");
    ASSERT_NE(client_ip, nullptr);
    EXPECT_EQ(client_ip->value, "1.1.1.1");

    const Token* terminating_rule_id = find_token(result, "terminating_rule_id");
    ASSERT_NE(terminating_rule_id, nullptr);
    EXPECT_EQ(terminating_rule_id->value, "STMTest_SQLi_XSS");

    const Token* request_id = find_token(result, "request_id");
    ASSERT_NE(request_id, nullptr);
    EXPECT_EQ(request_id->value, "aws-waf-request-001");
}

TEST(RuleLoader, LoadsExampleMessagesFromRules) {
    const std::vector<std::string> examples = RuleLoader::load_example_messages("rules");

    ASSERT_EQ(examples.size(), 3u);
    EXPECT_NE(std::find(examples.begin(), examples.end(), checkpoint_message()), examples.end());
    EXPECT_NE(std::find(examples.begin(), examples.end(), fortigate_message()), examples.end());
    EXPECT_NE(std::find(examples.begin(), examples.end(), aws_waf_message()), examples.end());
}
