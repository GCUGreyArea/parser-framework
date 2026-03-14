#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>

int main(int argc, const char ** argv) {
    google::InitGoogleLogging(argv[0]);
    testing::InitGoogleTest(&argc, (char**) argv);

    return RUN_ALL_TESTS();
}
