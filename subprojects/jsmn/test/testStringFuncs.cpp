#include <glog/logging.h>
#include <gtest/gtest.h>

#include "string_funcs.h"

#include <vector>
#include <string>

inline static std::string generate_huge_string(unsigned int size) {
    std::string ret = "";
    for (unsigned int index = 0; index < size; index++) {
        std::string str;
        for (int i = 0; i < 32; ++i) {
            int randomChar = rand() % (26 + 26 + 10);
            if (randomChar < 26)
                str += 'a' + randomChar;
            else if (randomChar < 26 + 26)
                str += 'A' + randomChar - 26;
            else
                str += '0' + randomChar - 26 - 26;
        }

        ret += str;
    }

    return ret;
}

TEST(testStringFuncs, testCMPIsGood) {
    const char *str1 = "This is a sring";
    char *str2 = copy_string((char *)str1);

    ASSERT_STREQ(str1, str2);
    ASSERT_TRUE(cmp_string((char *)str1, str2));

    kill_string(str2);
}

TEST(testStringFuncs, testBadCMP) {
    const char *str1 = "This is a string";
    const char *str2 = "This is a strinn";

    ASSERT_FALSE(cmp_string((char *)str1, (char *)str2));
}

TEST(testStringFuncs, testHugeString) {
    std::string str = generate_huge_string(10^125);
    char * str2 = copy_string((char*)str.c_str());

    ASSERT_STREQ(str.c_str(),str2);

    kill_string(str2);
}