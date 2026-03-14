#include <glog/logging.h>
#include <gtest/gtest.h>

#include <jsmn.hpp>
#include <set>

TEST(JSONPathParser, testJSONPathParser) {
    struct jqpath *path = NULL;
    const char *str = "$.name.value == \"new\"";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

    str = "$.name.value != \"test\"";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

    str = "$[\"name\"][1].value != \"test\"";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

    str = "$['name'][101].value == 12";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

    str = "$.name[1].value == 100.12";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);
}

TEST(JSONPathParser, testJSONPathDepth) {
    struct jqpath *path = NULL;
    struct jqpath saved = {};
    std::set<unsigned int> ints;

    const char *str = "$.name";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 1);
    auto single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    jqpath_close_path(path);

    str = "$.name.value";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 2);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    jqpath_close_path(path);

    str = "$['name'][1][\"value\"]";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 3);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    jqpath_close_path(path);

    str = "$[\"name\"][1]['value']['noun']";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 4);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    jqpath_close_path(path);

    str = "$[\"name\"][1][\"value\"]['three'][*]['four']";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 6);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    saved = *path;
    jqpath_close_path(path);

    str = "$['name'][00001][\"value\"]['three'][*]['four']";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 6);
    single = ints.emplace(path->hash);
    ASSERT_FALSE(single.second);
    ASSERT_EQ(saved.depth, path->depth);
    ASSERT_EQ(saved.hash, path->hash);
    jqpath_close_path(path);

    str = "$['name'][1][\"value\"]['three'][*]['four'] == 21";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 6);
    single = ints.emplace(path->hash);
    ASSERT_FALSE(single.second);
    ASSERT_EQ(path->op, JQ_EQUALS);
    ASSERT_EQ(path->value.value.int_val, 21);
    jqpath_close_path(path);

    str = "$['name'][1][\"value\"]['three'][*]['four'] != 21";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 6);
    single = ints.emplace(path->hash);
    ASSERT_FALSE(single.second);
    ASSERT_EQ(path->op, JQ_NOT_EQUALS);
    ASSERT_EQ(path->value.value.int_val, 21);
    jqpath_close_path(path);

    str = "$['name'][1][\"value\"]['three'][*]['four'] != \"Satan\"";
    path = jsonpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth, 6);
    single = ints.emplace(path->hash);
    ASSERT_FALSE(single.second);
    ASSERT_EQ(path->op, JQ_NOT_EQUALS);

    std::string val1("\"Satan\"");
    std::string val2(path->value.value.string_val);
    ASSERT_TRUE(val1 == val2);
    jqpath_close_path(path);
}

TEST(JSONPathParser, testJSONPathMatchesJQOutput) {
    struct jqpath *jq_path = jqpath_parse_string(".name[1].value.three[].four");
    struct jqpath *json_path =
        jsonpath_parse_string("$[\"name\"][1][\"value\"]['three'][*]['four']");

    ASSERT_TRUE(jq_path != NULL);
    ASSERT_TRUE(json_path != NULL);
    ASSERT_EQ(jq_path->depth, json_path->depth);
    ASSERT_EQ(jq_path->hash, json_path->hash);

    jqpath_close_path(jq_path);
    jqpath_close_path(json_path);
}

TEST(JSONPathParser, testDirectJSONPathParserRejectsJQSyntax) {
    struct jqpath *path = jsonpath_parse_string(".name[1]");
    ASSERT_TRUE(path == NULL);
}
