#include <glog/logging.h>
#include <gtest/gtest.h>

#include "jsmn.hpp"
#include "jqpath.h"

TEST(JSONPathJSMN, testJSONPathLookupWithRootSyntax) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = "$.name[2]";

    jsmn_parser p(json, 2);
    struct jqpath *path = jsonpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    auto *np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);
    ASSERT_TRUE(*np == "\"value 3\"");
}

TEST(JSONPathJSMN, testJSONPathLookupWithQuotedMembers) {
    std::string json = "{\"user_name\":{\"first name\":\"Barry\"}}";
    std::string path_str = "$['user_name'][\"first name\"]";

    jsmn_parser p(json, 2);
    struct jqpath *path = jsonpath_parse_string(path_str.c_str());

    int ret = p.parse();

    ASSERT_TRUE(ret > 0);
    ASSERT_TRUE(path != NULL);

    auto *np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);
    ASSERT_TRUE(*np == "\"Barry\"");
}

TEST(JSONPathJSMN, testAutoPathParserDispatchesJSONPath) {
    std::string json = "{\"name\":[\"value 1\",\"value 2\",\"value 3\"]}";
    std::string path_str = "$.name[1]";

    jsmn_parser p(json, 2);
    struct jqpath *path = path_parse_string(path_str.c_str());

    ASSERT_TRUE(p.parse() > 0);
    ASSERT_TRUE(path != NULL);

    auto *np = p.get_path(path);
    ASSERT_TRUE(np != nullptr);
    ASSERT_TRUE(*np == "\"value 2\"");
}
