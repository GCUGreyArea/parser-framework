#include <gtest/gtest.h>
#include <glog/logging.h>

#include "jsmn.hpp"
#include "string_funcs.h"
#include "JQ.hpp"

TEST(JQPathClass,testBasic) {
    struct jqpath * path = NULL;
    const char* str = ".name.value = \"new\"";
    path = jqpath_parse_string(str);

    JQ p(*path);

    ASSERT_TRUE(p == *path);
}

TEST(JQPathClass,testValueStringEquality) {
    struct jqpath * path = NULL;
    const char* str = ".name.value = \"new\"";
    path = jqpath_parse_string(str);

    JQ p(*path);

    ASSERT_TRUE(p == *path);

    ASSERT_TRUE(p == "\"new\"");
}

TEST(JQPathClass,testValueStdStringEquality) {
    struct jqpath * path = NULL;
    const char* str = ".name.value = \"new\"";
    path = jqpath_parse_string(str);

    JQ p(*path);

    ASSERT_TRUE(p == *path);
    std::string str1 = "\"new\"";

    ASSERT_TRUE(p == str1);
}

TEST(JQPathClass,testValueIntEquality) {
    struct jqpath * path = NULL;
    const char* str = ".name.value = 12";
    path = jqpath_parse_string(str);

    JQ p(*path);

    ASSERT_TRUE(p == *path);

    ASSERT_TRUE(p == 12);
}

TEST(JQPathClass,testValueFloatEquality) {
    struct jqpath * path = NULL;
    const char* str = ".name.value = 12.432";
    path = jqpath_parse_string(str);

    JQ p(*path);
    float flt = 12.432;

    ASSERT_TRUE(p == *path);

    ASSERT_TRUE(p == flt);
}
