#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
#include <libParser.h>


TEST(testPaths,testThatPathsGetBuilt) {
    std::vector<size_t> tokens = {0,1,2,3,4,5,6,7,8};

    /*
    ** Indicates a pattern based on inclusion
    ** 0 -> [0,1]
    ** 1 -> [1,2]
    ** 0.1 -> [1]
    ** 0.3 -> [0]
    ** 1.3 -> [2]
    */
    std::vector<std::vector<size_t>> patterns = {
        {0,3,6,7},
        {0,1,2,6},
        {1,3,8}
    };

    RootPath p(patterns);

    // Test paths against RootPath
    std::vector<size_t> pth = {0,1};
    std::vector v = p.path_has_pattern(pth);

    ASSERT_EQ(1,v.size());

    pth.clear();
    pth.push_back(0);
    pth.push_back(3);

    v = p.path_has_pattern(pth);

    ASSERT_EQ(1,v.size());

    pth.clear();
    pth.push_back(1);

    v = p.path_has_pattern(pth);

    ASSERT_EQ(2,v.size());

    pth.clear();
    pth.push_back(1);
    pth.push_back(3);

    v = p.path_has_tokens(pth);

    ASSERT_EQ(1,v.size());
    ASSERT_EQ(8,v.at(0));

    pth.push_back(8);

    v = p.path_has_pattern(pth);

    ASSERT_EQ(1,v.size());
    ASSERT_EQ(2,v.at(0));

    // Copy RootPath with copy constructor and retest
    RootPath p2(p);

    pth.clear();
    pth.push_back(0);
    pth.push_back(1);

    v = p2.path_has_pattern(pth);

    ASSERT_EQ(1,v.size());

    pth.clear();
    pth.push_back(0);
    pth.push_back(3);

    v = p2.path_has_pattern(pth);

    ASSERT_EQ(1,v.size());

    pth.clear();
    pth.push_back(1);

    v = p2.path_has_pattern(pth);

    ASSERT_EQ(2,v.size());

    pth.clear();
    pth.push_back(1);
    pth.push_back(3);

    v = p2.path_has_tokens(pth);

    ASSERT_EQ(1,v.size());
    ASSERT_EQ(8,v.at(0));

    pth.push_back(8);

    v = p2.path_has_pattern(pth);

    ASSERT_EQ(1,v.size());
    ASSERT_EQ(2,v.at(0));
}

TEST(testPaths, testPathsReportUniqueTokensAndMissingPaths) {
    std::vector<std::vector<size_t>> patterns = {
        {2, 0, 3},
        {1, 3},
        {4, 0}
    };

    RootPath root(patterns);

    std::vector<size_t> unique = root.get_unique_tokens();
    std::sort(unique.begin(), unique.end());

    ASSERT_EQ(unique, (std::vector<size_t>{0, 1, 2, 3, 4}));
    ASSERT_TRUE(root.path_has_pattern({9}).empty());
    ASSERT_TRUE(root.path_has_tokens({9}).empty());
}
