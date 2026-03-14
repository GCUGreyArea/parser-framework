#include <glog/logging.h>
#include <gtest/gtest.h>

#include "kv_state.h"

TEST(KVState, testKVState) {
    kv_state kv = kv_state::START;

    kv++;
    ASSERT_EQ(kv,kv_state::KEY);
    kv++;
    ASSERT_EQ(kv,kv_state::VALUE);
    kv++;
    ASSERT_EQ(kv,kv_state::NEXT);
}
