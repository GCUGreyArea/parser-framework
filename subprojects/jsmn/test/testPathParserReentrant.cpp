#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "jqpath.h"

namespace {

struct expected_path {
    unsigned int depth;
    unsigned int hash;
    enum jq_operator op;
    enum jqvaltype_e type;
    int int_value;
    const char *string_value;
};

expected_path build_expected(struct jqpath *path) {
    expected_path expected = {};
    expected.depth = path->depth;
    expected.hash = path->hash;
    expected.op = path->op;
    expected.type = path->value.type;
    expected.int_value = path->value.value.int_val;
    expected.string_value = path->value.type == JQ_STRING_VAL
                                ? path->value.value.string_val
                                : NULL;
    return expected;
}

bool matches_expected(struct jqpath *path, const expected_path &expected) {
    if (path == NULL) {
        return false;
    }

    if (path->depth != expected.depth || path->hash != expected.hash ||
        path->op != expected.op || path->value.type != expected.type) {
        return false;
    }

    if (expected.type == JQ_STRING_VAL) {
        return path->value.value.string_val != NULL &&
               expected.string_value != NULL &&
               std::string(path->value.value.string_val) == expected.string_value;
    }

    if (expected.type == JQ_INT_VAL) {
        return path->value.value.int_val == expected.int_value;
    }

    return true;
}

void run_parallel_parse_test(struct jqpath *(*parse_fn)(const char *),
                             const char *first, const expected_path &first_expected,
                             const char *second, const expected_path &second_expected) {
    std::atomic<bool> start(false);
    std::atomic<bool> failed(false);
    std::vector<std::thread> workers;

    for (int thread_idx = 0; thread_idx < 8; ++thread_idx) {
        workers.emplace_back([&, thread_idx]() {
            while (!start.load(std::memory_order_acquire)) {
            }

            for (int i = 0; i < 250; ++i) {
                const char *input = ((thread_idx + i) % 2 == 0) ? first : second;
                const expected_path &expected =
                    ((thread_idx + i) % 2 == 0) ? first_expected : second_expected;

                struct jqpath *path = parse_fn(input);
                if (!matches_expected(path, expected)) {
                    failed.store(true, std::memory_order_release);
                }
                jqpath_close_path(path);
            }
        });
    }

    start.store(true, std::memory_order_release);

    for (auto &worker : workers) {
        worker.join();
    }

    ASSERT_FALSE(failed.load(std::memory_order_acquire));
}

} // namespace

TEST(PathParserReentrant, testJQParserSupportsConcurrentCalls) {
    struct jqpath *first_path = jqpath_parse_string(".name[1].value != \"test\"");
    struct jqpath *second_path = jqpath_parse_string(".person[12].age = 42");

    ASSERT_TRUE(first_path != NULL);
    ASSERT_TRUE(second_path != NULL);

    expected_path first_expected = build_expected(first_path);
    expected_path second_expected = build_expected(second_path);

    run_parallel_parse_test(jqpath_parse_string, ".name[1].value != \"test\"",
                            first_expected, ".person[12].age = 42",
                            second_expected);

    jqpath_close_path(first_path);
    jqpath_close_path(second_path);
}

TEST(PathParserReentrant, testJSONPathParserSupportsConcurrentCalls) {
    struct jqpath *first_path =
        jsonpath_parse_string("$['name'][1]['value'] != \"test\"");
    struct jqpath *second_path =
        jsonpath_parse_string("$['person'][12]['age'] == 42");

    ASSERT_TRUE(first_path != NULL);
    ASSERT_TRUE(second_path != NULL);

    expected_path first_expected = build_expected(first_path);
    expected_path second_expected = build_expected(second_path);

    run_parallel_parse_test(jsonpath_parse_string,
                            "$['name'][1]['value'] != \"test\"", first_expected,
                            "$['person'][12]['age'] == 42", second_expected);

    jqpath_close_path(first_path);
    jqpath_close_path(second_path);
}
