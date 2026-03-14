#include <gtest/gtest.h>
#include <glog/logging.h>

#include <jsmn.hpp>
#include <set>

extern int lex_depth;

TEST(JQParser,testJQParser) {
    struct jqpath * path = NULL;
    const char* str = ".name.value = \"new\"";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

    str = ".name.value != \"test\"";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

    str = ".name[1].value != \"test\"";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

    str = ".name[101].value = 12";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

    str = ".name[1].value = 100.12";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    jqpath_close_path(path);

}

TEST(JQParser,testJQDepth) {
    struct jqpath* path = NULL;
    struct jqpath saved = {};

    // We also want to insure that each path results in a unique hash value
    std::set<unsigned int> ints;

    const char* str = ".name";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,1);
    auto single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    jqpath_close_path(path);

    str = ".name.value";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,2);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    jqpath_close_path(path);


    str = ".name[1].value";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,3);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    jqpath_close_path(path);

    str = ".name[1].value.noun";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,4);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    jqpath_close_path(path);

    str = ".name[1].value.three[].four";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,6);
    single = ints.emplace(path->hash);
    ASSERT_TRUE(single.second);
    saved = *path;
    jqpath_close_path(path);

    // The values should numerically be equivolet
    str = ".name[00001].value.three[].four";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,6);
    single = ints.emplace(path->hash);
    ASSERT_FALSE(single.second);

    ASSERT_EQ(saved.depth,path->depth);
    ASSERT_EQ(saved.hash,path->hash);
    jqpath_close_path(path);


    str = ".name[1].value.three[].four = 21";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,6);
    single = ints.emplace(path->hash);
    ASSERT_FALSE(single.second);

    ASSERT_EQ(path->op,JQ_EQUALS);
    ASSERT_EQ(path->value.value.int_val,21);
    jqpath_close_path(path);

    str = ".name[1].value.three[].four != 21";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,6);
    single = ints.emplace(path->hash);
    ASSERT_FALSE(single.second);

    ASSERT_EQ(path->op,JQ_NOT_EQUALS);
    ASSERT_EQ(path->value.value.int_val,21);
    jqpath_close_path(path);


    str = ".name[1].value.three[].four != \"Satan\"";
    path = jqpath_parse_string(str);
    ASSERT_TRUE(path != NULL);
    ASSERT_EQ(path->depth,6);
    single = ints.emplace(path->hash);
    ASSERT_FALSE(single.second);

    ASSERT_EQ(path->op,JQ_NOT_EQUALS);

    std::string val1("\"Satan\"");
    std::string val2(path->value.value.string_val);

    ASSERT_TRUE(val1 == val2);
    jqpath_close_path(path);

}

TEST(JQParser, testDirectJQParserRejectsJSONPathSyntax) {
    struct jqpath *path = jqpath_parse_string("$.name[1]");
    ASSERT_TRUE(path == NULL);
}

TEST(JQParser, testAutoPathParserDispatchesJQSyntax) {
    struct jqpath *direct = jqpath_parse_string(".name[1].value");
    struct jqpath *dispatched = path_parse_string(".name[1].value");

    ASSERT_TRUE(direct != NULL);
    ASSERT_TRUE(dispatched != NULL);
    ASSERT_EQ(direct->depth, dispatched->depth);
    ASSERT_EQ(direct->hash, dispatched->hash);

    jqpath_close_path(direct);
    jqpath_close_path(dispatched);
}
