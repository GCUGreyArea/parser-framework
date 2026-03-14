#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "DynamicProperty/Compiler.h"
#include "Output/HTMLOut.h"
#include "Utils/Numbers.h"
#include <libParser.h>

namespace {

Condition make_int_condition(int lhs_value, Condition::Operator op, int rhs_value) {
    BaseValue lhs(lhs_value);
    BaseValue rhs(rhs_value);
    return Condition(lhs, op, rhs);
}

std::unique_ptr<Rule> load_rule_for_framework_test(const std::string& path) {
    YamlFile fl(path);
    fl.readFile();
    return fl.getRule();
}

std::map<std::string, std::shared_ptr<Token>> make_dynamic_token_map(Match& match) {
    std::map<std::string, std::shared_ptr<Token>> tokens;

    auto detail = std::make_shared<Token>("detail", "detail=([A-Za-z]+)", ValueType::STRING, 0);
    auto context = std::make_shared<Token>("contextnum", "context=([0-9]+)", ValueType::INT, 1);

    match.set_match_type(Match::Type::SEQUENTIAL);
    EXPECT_TRUE(detail->match(match));
    EXPECT_TRUE(context->match(match));

    tokens.emplace("detail", detail);
    tokens.emplace("contextnum", context);
    return tokens;
}

} // namespace

TEST(testFrameworkEdges, testConditionSupportsDeferredRhsBinding) {
    BaseValue lhs;
    BaseValue rhs;
    Condition condition(lhs, Condition::EQUAL_TO, rhs);

    condition.set_lhs("same");
    condition.set_rhs("same");

    ASSERT_TRUE(condition.valid());
    ASSERT_TRUE(condition.evaluate());
}

TEST(testFrameworkEdges, testConditionStringContainsMatchesAtStartAndRejectsMissingText) {
    BaseValue source("prefix-value");
    BaseValue starts_with("prefix");
    BaseValue missing("missing");

    Condition matches_start(source, Condition::STR_CONTAINS, starts_with);
    Condition misses(source, Condition::STR_CONTAINS, missing);

    ASSERT_TRUE(matches_start.evaluate());
    ASSERT_FALSE(misses.evaluate());
}

TEST(testFrameworkEdges, testBoolExpressionOrEvaluatesToTrueWhenEitherSideMatches) {
    Condition true_condition = make_int_condition(5, Condition::EQUAL_TO, 5);
    Condition false_condition = make_int_condition(5, Condition::EQUAL_TO, 4);

    BoolExpression direct_or(true_condition, BoolExpression::OR, false_condition);
    ASSERT_TRUE(direct_or.evaluate());

    std::shared_ptr<BoolExpression> nested_true = std::make_shared<BoolExpression>(true_condition);
    BoolExpression nested_or(false_condition, BoolExpression::OR, nested_true);
    ASSERT_TRUE(nested_or.evaluate());
}

TEST(testFrameworkEdges, testTokenContainsUsesSubstringSemantics) {
    Token token("vendor", "vendor=([a-z]+)", ValueType::STRING, 0);
    Match match("vendor=aws");
    match.set_match_type(Match::Type::SEQUENTIAL);

    ASSERT_TRUE(token.match(match));
    ASSERT_TRUE(token.contains("aws"));
    ASSERT_FALSE(token.contains("azure"));
}

TEST(testFrameworkEdges, testAnchorPatternValidationAndRegexDescription) {
    AnchorPattern missing_id("anchor", "");
    ASSERT_THROW(missing_id.validate(), Exception::NoId);

    AnchorPattern missing_name("", "ANCHOR-ID");
    ASSERT_THROW(missing_name.validate(), Exception::NoName);

    AnchorPattern combined("anchor", "ANCHOR-ID", std::vector<std::string>{"^msg", "^msg", "action=.*"});
    ASSERT_EQ(combined.get_regex_pattern(), "^msg.*?action=.*");
}

TEST(testFrameworkEdges, testBaseValueConvertsBetweenStringAndIntegerRepresentations) {
    BaseValue int_target(ValueType::INT);
    BaseValue hex_source(std::string("0x10"));
    int_target = hex_source;
    ASSERT_EQ(int_target.as_int(), 16);

    BaseValue string_target(ValueType::STRING);
    BaseValue int_source(27);
    string_target = int_source;
    ASSERT_EQ(string_target.as_string(), "27");
}

TEST(testFrameworkEdges, testBaseValueBackedByTokenCannotBeAssigned) {
    Token token("vendor", "vendor=([a-z]+)", ValueType::STRING, 0);
    Match match("vendor=aws");
    match.set_match_type(Match::Type::SEQUENTIAL);

    ASSERT_TRUE(token.match(match));

    BaseValue token_value(std::make_shared<Token>(token));
    BaseValue other_value("azure");
    ASSERT_THROW(token_value = other_value, Exception::CantSetConstVariable);
}

TEST(testFrameworkEdges, testMatchResetClearsOffsetPatternAndType) {
    AnchorPattern anchor("anchor", "ANCHOR-ID", "^(msg)");
    Pattern pattern("pattern", "PATTERN-ID", anchor, 1);

    Match match("msg");
    match.moveTo(3);
    match.set_match_type(Match::Type::SEQUENTIAL);
    match.setMatch(&pattern);

    ASSERT_TRUE(match.end_of_message());

    match.reset();

    ASSERT_EQ(match.offset(), 0u);
    ASSERT_EQ(match.type(), Match::Type::NONE);
    ASSERT_EQ(match.getMathcedPattern(), nullptr);
}

TEST(testFrameworkEdges, testRegexAssertAndRandomMatchPathsWork) {
    Regex vendor("vendor=([a-z]+)");
    Match asserted("vendor=aws vendor=azure");
    asserted.set_match_type(Match::Type::SEQUENTIAL);

    ASSERT_TRUE(vendor.match(asserted, "azure"));
    ASSERT_EQ(vendor.as_string(), "azure");

    Regex device("device=([a-z]+)");
    Match random("msg context=5 device=toy vendor=aws");
    random.set_match_type(Match::Type::RANDOM);

    ASSERT_TRUE(device.match(random));
    ASSERT_EQ(device.as_string(), "toy");
}

TEST(testFrameworkEdges, testDynamicCompilerEvaluatesContainsAndDivision) {
    Match match("detail=security context=8");
    auto token_map = make_dynamic_token_map(match);

    std::shared_ptr<ConditionalAssignment> compiled = DynamicProperty::compile(
        "if detail ?= \"cur\" then score = contextnum / 2 else score = 0",
        token_map
    );

    ASSERT_TRUE(compiled->evaluate());

    const auto& results = compiled->get_results();
    ASSERT_EQ(results.size(), 1u);
    ASSERT_EQ(results[0].name, "score");
    ASSERT_EQ(results[0].value, "4");
}

TEST(testFrameworkEdges, testDynamicCompilerRejectsUnexpectedCharacters) {
    Match match("detail=security context=8");
    auto token_map = make_dynamic_token_map(match);

    ASSERT_THROW(
        DynamicProperty::compile("if detail == @ then score = 1", token_map),
        Exception::General
    );
}

TEST(testOutput, testJsonOutWritesNullRuleWhenNothingMatched) {
    std::unique_ptr<Rule> rule = load_rule_for_framework_test("test/resources/properties.yaml");
    Match match("other action=Accept vendor=aws device=toy context=5");

    ASSERT_EQ(rule->match(match), nullptr);

    JSONOut out;
    out.add_rule_value(*rule);

    std::stringstream stream;
    stream << out;

    nlohmann::json json = nlohmann::json::parse(stream.str());
    ASSERT_EQ(json["parsed"][0]["rule"], "null");
}

TEST(testOutput, testHtmlOutPlaceholderRendersToStream) {
    HTMLOut out;
    std::stringstream stream;
    stream << out;

    ASSERT_NE(stream.str().find("NOT IMPLIMETED YET!"), std::string::npos);
}

TEST(testRule, testAnchorRegexDescriptionsAreDeduplicatedAcrossPatterns) {
    std::unique_ptr<Rule> rule = load_rule_for_framework_test("test/resources/properties.yaml");

    std::vector<std::string> regexes = rule->get_anchor_regex();

    ASSERT_EQ(regexes.size(), 1u);
    ASSERT_EQ(regexes[0], "^(msg)");
}

TEST(testNumbers, testNumbersRecognizeAndConvertCommonBases) {
    ASSERT_EQ(Numbers::is_number("15"), 10u);
    ASSERT_EQ(Numbers::is_number("017"), 8u);
    ASSERT_EQ(Numbers::is_number("0b1010"), 2u);
    ASSERT_EQ(Numbers::is_number("0x1f"), 16u);
    ASSERT_EQ(Numbers::is_number("0b102"), 0u);

    ASSERT_EQ(Numbers::to_int("15", 10), 15);
    ASSERT_EQ(Numbers::to_int("017", 8), 15);
    ASSERT_EQ(Numbers::to_int("0b1010", 2), 10);
    ASSERT_EQ(Numbers::to_int("0x1f", 16), 31);
}
