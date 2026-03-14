#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Exception/Exceptions.h"
#include <libParser.h>

namespace {

std::unique_ptr<Rule> load_rule(const std::string& path) {
    YamlFile fl(path);
    fl.readFile();
    return fl.getRule();
}

std::vector<std::unique_ptr<Rule>> load_rules(const std::vector<std::string>& paths) {
    std::vector<std::unique_ptr<Rule>> rules;
    for (const auto& path : paths) {
        YamlFile fl(path);
        fl.readFile();
        rules.push_back(fl.getRule());
    }

    return rules;
}

std::unordered_map<std::string, std::string> to_map(const std::vector<ResolvedProperty>& properties) {
    std::unordered_map<std::string, std::string> values;
    for(const auto& property : properties) {
        values.emplace(property.name, property.value);
    }

    return values;
}

std::vector<std::shared_ptr<Token>> make_random_test_tokens() {
    return {
        std::make_shared<Token>("vendor", "vendor=([a-z]+)", ValueType::STRING, 0),
        std::make_shared<Token>("device_id", "device=([a-z]+)", ValueType::STRING, 1),
        std::make_shared<Token>("contextnum", "context=([0-9]+)", ValueType::INT, 2),
    };
}

} // namespace

TEST(testProperties, testResolvedPropertiesRefreshAcrossMatchesAndAreWrittenToJson) {
    constexpr const char * FIRST_MSG = "msg action=Accept vendor=aws device=toy context=5";
    constexpr const char * SECOND_MSG = "msg action=Accept vendor=azure device=box context=1";

    std::unique_ptr<Rule> rule = load_rule("test/resources/properties.yaml");

    Match first(FIRST_MSG);
    ASSERT_TRUE(rule->match(first));

    Pattern * first_pattern = rule->matched();
    ASSERT_NE(first_pattern, nullptr);
    ASSERT_EQ(first_pattern->name(), "Accepted Event");

    auto first_properties = to_map(first_pattern->get_properties());
    ASSERT_EQ(first_properties["event.type"], "aws.toy");
    ASSERT_EQ(first_properties["threat.level"], "danger");

    JSONOut out;
    out.add_rule_value(*rule);

    Match second(SECOND_MSG);
    ASSERT_TRUE(rule->match(second));

    Pattern * second_pattern = rule->matched();
    ASSERT_NE(second_pattern, nullptr);
    ASSERT_EQ(second_pattern->name(), "Accepted Event");

    auto second_properties = to_map(second_pattern->get_properties());
    ASSERT_EQ(second_properties["event.type"], "azure.box");
    ASSERT_EQ(second_properties["threat.level"], "low");

    out.add_rule_value(*rule);

    std::stringstream stream;
    stream << out;

    nlohmann::json json = nlohmann::json::parse(stream.str());
    ASSERT_EQ(json["parsed"][0]["property"]["event.type"], "aws.toy");
    ASSERT_EQ(json["parsed"][0]["property"]["threat.level"], "danger");
    ASSERT_EQ(json["parsed"][1]["property"]["event.type"], "azure.box");
    ASSERT_EQ(json["parsed"][1]["property"]["threat.level"], "low");
}

TEST(testProperties, testDynamicGrammarSupportsPrecedenceParenthesesAndElseAssignments) {
    std::unique_ptr<Rule> rule = load_rule("test/resources/dynamic_grammar.yaml");

    Match first("msg action=Accept vendor=aws device=toy context=5");
    ASSERT_TRUE(rule->match(first));

    Pattern * pattern = rule->matched();
    ASSERT_NE(pattern, nullptr);
    auto first_properties = to_map(pattern->get_properties());
    ASSERT_EQ(first_properties["event.signature"], "aws.toy");
    ASSERT_EQ(first_properties["score"], "11");

    Match second("msg action=Accept vendor=azure device=box context=1");
    ASSERT_TRUE(rule->match(second));

    pattern = rule->matched();
    ASSERT_NE(pattern, nullptr);
    auto second_properties = to_map(pattern->get_properties());
    ASSERT_EQ(second_properties["event.signature"], "azure.box");
    ASSERT_EQ(second_properties["score"], "7");

    Match third("msg action=Reject vendor=aws device=toy context=1");
    ASSERT_TRUE(rule->match(third));

    pattern = rule->matched();
    ASSERT_NE(pattern, nullptr);
    auto third_properties = to_map(pattern->get_properties());
    ASSERT_EQ(third_properties["event.signature"], "aws.fallback");
    ASSERT_EQ(third_properties["score"], "9");
}

TEST(testPatternModes, testRandomMatchModeAllowsOutOfOrderTokens) {
    constexpr const char * ANCHOR_ID = "A-RANDOM-ANCHOR";
    constexpr const char * PATTERN_ID = "A-RANDOM-PATTERN";
    constexpr const char * MESSAGE = "msg context=5 device=toy vendor=aws";

    AnchorPattern anchor("anchor", ANCHOR_ID, "^(msg)");

    Pattern sequential("sequential", PATTERN_ID, anchor, make_random_test_tokens(), {}, {}, 1, Match::Type::SEQUENTIAL);
    Match sequential_match(MESSAGE);
    ASSERT_FALSE(sequential.match(sequential_match));

    Pattern random("random", PATTERN_ID, anchor, make_random_test_tokens(), {}, {}, 1, Match::Type::RANDOM);
    Match random_match(MESSAGE);
    ASSERT_TRUE(random.match(random_match));

    std::vector<std::shared_ptr<Token>>& matched_tokens = random.get_tokens();
    ASSERT_EQ(matched_tokens[0]->as_string(), "aws");
    ASSERT_EQ(matched_tokens[1]->as_string(), "toy");
    ASSERT_EQ(matched_tokens[2]->as_int(), 5);
}

TEST(testPatternOrdering, testLowerPrecedenceValueWinsWhenTokenCountsMatch) {
    std::unique_ptr<Rule> rule = load_rule("test/resources/precedence_equal_tokens.yaml");

    Match match("msg kind=event");
    ASSERT_TRUE(rule->match(match));

    Pattern * pattern = rule->matched();
    ASSERT_NE(pattern, nullptr);
    ASSERT_EQ(pattern->id(), "Z-LOW-PRECEDENCE");
    ASSERT_EQ(pattern->name(), "Low Precedence Value");
}

TEST(testPatternOrdering, testMoreSpecificPatternWinsBeforePrecedenceComparison) {
    std::unique_ptr<Rule> rule = load_rule("test/resources/precedence_token_count.yaml");

    Match match("msg kind=event detail=full");
    ASSERT_TRUE(rule->match(match));

    Pattern * pattern = rule->matched();
    ASSERT_NE(pattern, nullptr);
    ASSERT_EQ(pattern->id(), "Z-SPECIFIC");
    ASSERT_EQ(pattern->name(), "Specific Match");
}

TEST(testYamlErrors, testMissingTokenThrows) {
    YamlFile fl("test/resources/invalid_missing_token.yaml");
    ASSERT_THROW(fl.readFile(), Exception::TokenNotFound);
}

TEST(testYamlErrors, testUnknownMatchTypeThrows) {
    YamlFile fl("test/resources/invalid_match_type.yaml");
    ASSERT_THROW(fl.readFile(), Exception::UnknownMatchType);
}

TEST(testYamlExamples, testExamplesValidateTokensPropertiesAndAbsentProperties) {
    std::vector<std::unique_ptr<Rule>> rules = load_rules({"test/resources/examples_valid.yaml"});
    ASSERT_NO_THROW(RuleValidation::validate_examples(rules));
}

TEST(testYamlExamples, testExampleMustMatchItsEnclosingPattern) {
    std::vector<std::unique_ptr<Rule>> rules = load_rules({"test/resources/invalid_example_exact_pattern.yaml"});
    ASSERT_THROW(RuleValidation::validate_examples(rules), Exception::ExampleMessageDoesntMatch);
}

TEST(testYamlExamples, testExamplePropertyExpectationsAreValidated) {
    std::vector<std::unique_ptr<Rule>> rules = load_rules({"test/resources/invalid_example_property.yaml"});
    ASSERT_THROW(RuleValidation::validate_examples(rules), Exception::ExampleMessageDoesntMatch);
}

TEST(testYamlExamples, testExamplesDetectCrossRuleConflicts) {
    std::vector<std::unique_ptr<Rule>> rules = load_rules({
        "test/resources/examples_valid.yaml",
        "test/resources/examples_conflicting_rule.yaml"
    });
    ASSERT_THROW(RuleValidation::validate_examples(rules), Exception::ExampleMessageDoesntMatch);
}

TEST(testRuleLoader, testLoadRulesLoadsAndValidatesDirectory) {
    ASSERT_NO_THROW({
        std::vector<std::unique_ptr<Rule>> rules = RuleLoader::load_rules("test/resources/rule_loader_valid");
        ASSERT_EQ(rules.size(), 1u);
    });
}

TEST(testRuleLoader, testLoadRulesRejectsConflictingDirectory) {
    ASSERT_THROW(
        RuleLoader::load_rules("test/resources/rule_loader_conflict"),
        Exception::General
    );
}

TEST(testRuleLoader, testLoadRulesLoadsBundledRulesDirectory) {
    ASSERT_NO_THROW({
        std::vector<std::unique_ptr<Rule>> rules = RuleLoader::load_rules("rules");
        ASSERT_EQ(rules.size(), 2u);
    });
}

TEST(testYamlErrors, testInvalidDynamicExpressionThrows) {
    YamlFile fl("test/resources/invalid_dynamic.yaml");
    ASSERT_THROW(fl.readFile(), Exception::General);
}

TEST(testYamlErrors, testUnknownIdentifierInDynamicExpressionThrows) {
    YamlFile fl("test/resources/invalid_unknown_identifier.yaml");
    ASSERT_THROW(fl.readFile(), Exception::General);
}
