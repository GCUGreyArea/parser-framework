#include <algorithm>
#include <string>

#include <gtest/gtest.h>

#include "parser_framework/IngestionBundle.hpp"
#include "parser_framework/IngestionPipeline.hpp"
#include "parser_framework/MessageLoader.hpp"
#include "parser_framework/ParserFramework.hpp"
#include "parser_framework/ReportAnalyzer.hpp"
#include "parser_framework/RuleLoader.hpp"
#include "parser_framework/utils/Args.hpp"

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
    return MessageLoader::load_message_file("messages/checkpoint/checkpoint-accept.log");
}

std::string fortigate_message() {
    return MessageLoader::load_message_file("messages/fortigate/traffic-close.log");
}

std::string fortigate_ips_message() {
    return MessageLoader::load_message_file("messages/fortigate/ips-log4shell.log");
}

std::string aws_waf_message() {
    return MessageLoader::load_message_file("messages/aws-waf/sqli-block.json");
}

std::string cloudflare_message() {
    return MessageLoader::load_message_file("messages/cloudflare/log4shell-header-block.json");
}

std::string suricata_message() {
    return MessageLoader::load_message_file("messages/suricata/cryptfile2-alert.json");
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

TEST(Args, LoadsSingleAndMultiValueFlags) {
    const char* argv[] = {
        "example_parser",
        "--rules", "custom-rules",
        "--messages", "messages/aws-waf/sqli-block.json",
        "messages/cloudflare/log4shell-header-block.json",
        "-m", "messages/suricata/cryptfile2-alert.json"
    };

    utils::Args args(static_cast<int>(sizeof(argv) / sizeof(argv[0])), argv);
    args.add_string_value("-r", "--rules", "rules");
    args.add_string_values("-m", "--messages", {"messages"});

    EXPECT_EQ(args.get_string_value("-r"), "custom-rules");

    const std::vector<std::string> messages = args.get_string_values("-m");
    ASSERT_EQ(messages.size(), 3u);
    EXPECT_EQ(messages[0], "messages/aws-waf/sqli-block.json");
    EXPECT_EQ(messages[1], "messages/cloudflare/log4shell-header-block.json");
    EXPECT_EQ(messages[2], "messages/suricata/cryptfile2-alert.json");
}

TEST(RuleLoader, LoadsCheckPointRulesAndParsesMessage) {
    const std::vector<MessageRule> rules = RuleLoader::load_rules("rules");
    ASSERT_EQ(rules.size(), 6u);

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

TEST(RuleLoader, LoadsFortiGateIpsRulesAndParsesMessage) {
    ParserFramework framework(RuleLoader::load_rules("rules"));
    ParseResult result = framework.parse_message(fortigate_ips_message());

    ASSERT_TRUE(result.matched);
    ASSERT_TRUE(result.errors.empty());
    EXPECT_EQ(result.message_rule_name, "FortiGate IPS");
    ASSERT_EQ(result.token_extraction.size(), 2u);
    EXPECT_EQ(result.token_extraction[0], "5d6ce0b2-55fd-4bb8-b376-786b63d79795");
    EXPECT_EQ(result.token_extraction[1], "2a1f7d47-0f1b-4d7b-9cf7-4f118ec93241");

    const Token* srcip = find_token(result, "srcip");
    ASSERT_NE(srcip, nullptr);
    EXPECT_EQ(srcip->value, "198.51.100.25");

    const Token* attack = find_token(result, "attack");
    ASSERT_NE(attack, nullptr);
    EXPECT_EQ(attack->value, "Apache.Log4j.Error.Log.Remote.Code.Execution");

    EXPECT_EQ(result.properties.at("threat.event"), "ips_detection");
    EXPECT_EQ(result.properties.at("threat.source_ip"), "198.51.100.25");
    EXPECT_EQ(result.properties.at("threat.destination_ip"), "10.0.0.25");
    EXPECT_EQ(result.properties.at("threat.attack"), "Apache.Log4j.Error.Log.Remote.Code.Execution");
    EXPECT_EQ(result.properties.at("threat.severity"), "critical");
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

    EXPECT_EQ(result.properties.at("threat.event"), "web_attack");
    EXPECT_EQ(result.properties.at("threat.source_ip"), "1.1.1.1");
    EXPECT_EQ(result.properties.at("threat.rule_name"), "STMTest_SQLi_XSS");
    EXPECT_EQ(result.properties.at("threat.outcome"), "BLOCK");
    EXPECT_EQ(result.properties.at("threat.family"), "sqli");
    EXPECT_EQ(result.properties.at("threat.technique_id"), "T1190");
    EXPECT_EQ(result.properties.at("threat.tactic_id"), "TA0001");
    EXPECT_EQ(result.properties.at("threat.system"), "aws_waf");
}

TEST(RuleLoader, LoadsCloudflareRulesAndParsesMessage) {
    ParserFramework framework(RuleLoader::load_rules("rules"));
    ParseResult result = framework.parse_message(cloudflare_message());

    ASSERT_TRUE(result.matched);
    ASSERT_TRUE(result.errors.empty());
    EXPECT_EQ(result.message_rule_name, "Cloudflare Firewall Event");
    ASSERT_EQ(result.token_extraction.size(), 2u);
    EXPECT_EQ(result.token_extraction[0], "3fb1b277-7ec1-4cb9-a4f8-e9a87f9509d1");
    EXPECT_EQ(result.token_extraction[1], "a8f4f3d2-12be-4f3a-a1e4-2a4a2b35bcb2");

    const Token* action = find_token(result, "action");
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->value, "block");

    const Token* client_ip = find_token(result, "client_ip");
    ASSERT_NE(client_ip, nullptr);
    EXPECT_EQ(client_ip->value, "198.51.100.25");

    const Token* rule_id = find_token(result, "rule_id");
    ASSERT_NE(rule_id, nullptr);
    EXPECT_EQ(rule_id->value, "6b1cc72dff9746469d4695a474430f12");

    EXPECT_EQ(result.properties.at("threat.event"), "web_attack");
    EXPECT_EQ(result.properties.at("threat.source_ip"), "198.51.100.25");
    EXPECT_EQ(result.properties.at("threat.family"), "log4shell");
    EXPECT_EQ(result.properties.at("threat.technique_id"), "T1190");
    EXPECT_EQ(result.properties.at("threat.tactic_id"), "TA0001");
    EXPECT_EQ(result.properties.at("threat.system"), "cloudflare_waf");
}

TEST(RuleLoader, LoadsSuricataRulesAndParsesMessage) {
    ParserFramework framework(RuleLoader::load_rules("rules"));
    ParseResult result = framework.parse_message(suricata_message());

    ASSERT_TRUE(result.matched);
    ASSERT_TRUE(result.errors.empty());
    EXPECT_EQ(result.message_rule_name, "Suricata EVE Alert");
    ASSERT_EQ(result.token_extraction.size(), 2u);
    EXPECT_EQ(result.token_extraction[0], "d6874d8d-1c8a-4ff8-a686-7608e35f3371");
    EXPECT_EQ(result.token_extraction[1], "5b53b8da-8760-4e71-b4bc-62f7d76e37d3");

    const Token* src_ip = find_token(result, "src_ip");
    ASSERT_NE(src_ip, nullptr);
    EXPECT_EQ(src_ip->value, "142.11.240.191");

    const Token* alert_signature = find_token(result, "alert_signature");
    ASSERT_NE(alert_signature, nullptr);
    EXPECT_EQ(alert_signature->value, "ET MALWARE Win32/CryptFile2 / Revenge Ransomware Checkin M3");

    EXPECT_EQ(result.properties.at("threat.event"), "post_compromise_activity");
    EXPECT_EQ(result.properties.at("threat.source_ip"), "142.11.240.191");
    EXPECT_EQ(result.properties.at("threat.destination_ip"), "192.0.2.55");
    EXPECT_EQ(result.properties.at("threat.signature"), "ET MALWARE Win32/CryptFile2 / Revenge Ransomware Checkin M3");
    EXPECT_EQ(result.properties.at("threat.severity"), "1");
    EXPECT_EQ(result.properties.at("threat.family"), "cryptfile2");
    EXPECT_EQ(result.properties.at("threat.technique_id"), "T1071");
    EXPECT_EQ(result.properties.at("threat.tactic_id"), "TA0011");
    EXPECT_EQ(result.properties.at("threat.system"), "suricata");
}

TEST(RuleLoader, LoadsExampleMessagesFromRules) {
    const std::vector<std::string> examples = RuleLoader::load_example_messages("rules");

    ASSERT_EQ(examples.size(), 6u);
    EXPECT_NE(std::find(examples.begin(), examples.end(), checkpoint_message()), examples.end());
    EXPECT_NE(std::find(examples.begin(), examples.end(), fortigate_message()), examples.end());
    EXPECT_NE(std::find(examples.begin(), examples.end(), aws_waf_message()), examples.end());
    EXPECT_NE(std::find(examples.begin(), examples.end(), cloudflare_message()), examples.end());
    EXPECT_NE(std::find(examples.begin(), examples.end(), fortigate_ips_message()), examples.end());
    EXPECT_NE(std::find(examples.begin(), examples.end(), suricata_message()), examples.end());
}

TEST(ReportAnalyzer, GeneratesBreachReportsFromParsedOutput) {
    ParserFramework framework(RuleLoader::load_rules("rules"));
    ReportAnalyzer analyzer(ReportRuleLoader::load_rules("report_rules"));

    const std::vector<ParseResult> results = framework.parse_messages({
        cloudflare_message(),
        aws_waf_message(),
        fortigate_ips_message(),
        suricata_message(),
        fortigate_message(),
        checkpoint_message()
    });

    const std::vector<ReportFinding> reports = analyzer.analyze(results);
    ASSERT_EQ(reports.size(), 5u);

    const std::string json = render_reports_as_json(reports);
    EXPECT_NE(json.find("\"phase\": \"initial_access\""), std::string::npos);
    EXPECT_NE(json.find("\"phase\": \"exploitation\""), std::string::npos);
    EXPECT_NE(json.find("\"phase\": \"command_and_control\""), std::string::npos);
    EXPECT_NE(json.find("\"event\": \"multi_system_breach_attempt\""), std::string::npos);
    EXPECT_NE(json.find("\"source\": {\n        \"message_rule_id\": \"1c507cb9-91d4-4024-871d-49f38ee08f36\""), std::string::npos);
    EXPECT_NE(json.find("\"message_rule_name\": \"Cloudflare Firewall Event\""), std::string::npos);
    EXPECT_NE(json.find("\"attack\": {\n          \"family\": \"log4shell\",\n          \"name\": \"Apache.Log4j.Error.Log.Remote.Code.Execution\""), std::string::npos);
    EXPECT_NE(json.find("\"family\": \"log4shell\""), std::string::npos);
    EXPECT_NE(json.find("\"technique\": {\n          \"id\": \"T1190\""), std::string::npos);
    EXPECT_NE(json.find("\"sources\": ["), std::string::npos);
    EXPECT_NE(json.find("\"systems\": {\n          \"count\": \"2\""), std::string::npos);
    EXPECT_NE(json.find("\"system\": \"suricata\""), std::string::npos);
    EXPECT_EQ(json.find("\"report\": {\n        \"report\":"), std::string::npos);
}

TEST(IngestionLoader, LoadsAttributedBundle) {
    const IngestionBundle bundle = IngestionLoader::load_bundle("bundles/multi-tenant-ingestion.json");

    EXPECT_EQ(bundle.schema_version, "1.0.0");
    EXPECT_EQ(bundle.storage.backend, "mongodb");
    ASSERT_EQ(bundle.organizations.size(), 4u);
    ASSERT_EQ(bundle.networks.size(), 3u);
    ASSERT_EQ(bundle.systems.size(), 4u);
    ASSERT_EQ(bundle.collections.size(), 4u);
    EXPECT_EQ(bundle.collections[0].system_id, "sys-cloudflare-waf");
    EXPECT_EQ(bundle.collections[2].attribution.operator_org_ids[0], "org-bae");
    EXPECT_EQ(bundle.systems[1].attribution.provider_org_ids[0], "org-aws");
}

TEST(IngestionPipeline, ProcessesBundleAndGeneratesBundleReports) {
    const IngestionBundle bundle = IngestionLoader::load_bundle("bundles/multi-tenant-ingestion.json");
    IngestionPipeline pipeline(
        ParserFramework(RuleLoader::load_rules("rules")),
        ReportAnalyzer(ReportRuleLoader::load_rules("report_rules")));

    const BundleProcessingResult result = pipeline.process(bundle);
    ASSERT_EQ(result.collections.size(), 4u);
    ASSERT_EQ(result.reports.size(), 5u);

    EXPECT_TRUE(result.collections[0].system.has_value());
    EXPECT_TRUE(result.collections[0].network.has_value());
    EXPECT_TRUE(result.collections[0].site.has_value());
    ASSERT_EQ(result.collections[0].parsed.size(), 1u);
    EXPECT_EQ(result.collections[0].parsed[0].message_rule_name, "Cloudflare Firewall Event");

    const std::string json = render_bundle_processing_result_as_json(result);
    EXPECT_NE(json.find("\"bundle_id\": \"bundle-2026-03-15-demo-001\""), std::string::npos);
    EXPECT_NE(json.find("\"backend\": \"mongodb\""), std::string::npos);
    EXPECT_NE(json.find("\"name\": \"Customer Secure Network\""), std::string::npos);
    EXPECT_NE(json.find("\"event\": \"multi_system_breach_attempt\""), std::string::npos);
    EXPECT_NE(json.find("\"systems\": {"), std::string::npos);
    EXPECT_NE(json.find("\"count\": \"2\""), std::string::npos);
}
