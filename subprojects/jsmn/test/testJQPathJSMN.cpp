#include <glog/logging.h>
#include <gtest/gtest.h>
#include <cstdio>

#include "jsmn.hpp"
#include "jqpath.h"


TEST(JQPathJSMN, testJSMNJQPathStringValues) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = ".name[1] = \"value 2\"";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"value 2\"");
}

TEST(JQPathJSMN, testJSMNJQPathOpenArrayStringValues) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = ".name[1] = \"value 2\"";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"value 2\"");
}

TEST(JQPathJSMN, testJSMNJQPathOpenArrayStringValuesComplex) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = ".name[2] = \"value 3\"";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"value 3\"");
}

TEST(JQPathJSMN, testJSMNJQPathOpenArrayStringValuesComplexWithoutEquals) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = ".name[2]";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"value 3\"");
}

TEST(JQPathJSMN, testJSMNJQPathOpenArrayStringValuesSimpleWithoutEquals) {
    std::string json = "{\"name\":\"Barry\"}";
    std::string path_str = ".name";


    jsmn_parser p(json,2);

    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    auto * np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);

    ASSERT_TRUE(*np == "\"Barry\"");
}

TEST(JQPathJSMN, testJSMNJQPathLookupAfterDeserialise) {
    std::string json = "{\"name\":\"Barry\"}";
    std::string path_str = ".name";
    const char * serialised_file = "test/resources/jqpath-out.bin";

    jsmn_parser p(json, 2);
    struct jqpath * path = jqpath_parse_string(path_str.c_str());

    ASSERT_TRUE(p.parse() > 0);
    ASSERT_TRUE(path != NULL);
    ASSERT_TRUE(p.serialise(serialised_file));

    jsmn_parser p2;
    ASSERT_TRUE(p2.deserialise(serialised_file));

    auto * np = p2.get_path(path);
    ASSERT_TRUE(np != nullptr);
    ASSERT_TRUE(*np == "\"Barry\"");

    std::remove(serialised_file);
}
