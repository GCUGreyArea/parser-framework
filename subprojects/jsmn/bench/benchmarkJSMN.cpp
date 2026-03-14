#include <benchmark/benchmark.h>

#include <cstdint>
#include <stdexcept>
#include <string>

#include "benchmark_fixtures.h"
#include "jsmn.hpp"
#include "jqpath.h"

#ifdef HAVE_SIMDJSON
#include <simdjson.h>
#endif

#ifdef HAVE_YYJSON
#include <yyjson.h>
#endif

#ifdef HAVE_RAPIDJSON
#include <rapidjson/document.h>
#endif

namespace {

std::int64_t processed_bytes(const std::string &json,
                             benchmark::State &state) {
    return static_cast<std::int64_t>(json.size()) *
           static_cast<std::int64_t>(state.iterations());
}

class jqpath_ptr {
  public:
    explicit jqpath_ptr(struct jqpath *path) : m_path(path) {}
    ~jqpath_ptr() { jqpath_close_path(m_path); }

    struct jqpath *get() const { return m_path; }

  private:
    struct jqpath *m_path;
};

static void BM_JSMNTokenizeOnlySmallDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::small_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        benchmark::DoNotOptimize(parser.parse());
    }

    state.SetBytesProcessed(processed_bytes(json, state));
}

static void BM_JSMNTokenizeOnlyLargeDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::large_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        benchmark::DoNotOptimize(parser.parse());
    }

    state.SetBytesProcessed(processed_bytes(json, state));
}

static void BM_JSMNParseAndIndexLargeDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::large_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        benchmark::DoNotOptimize(parser.parse());
        parser.render();
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(processed_bytes(json, state));
}

static void BM_RebuildPathsMediumDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::medium_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        if (parser.parse() < 0) {
            throw std::runtime_error("failed to parse medium benchmark fixture");
        }
        parser.render();
        benchmark::ClobberMemory();
    }
}

#ifdef HAVE_SIMDJSON
static void BM_SimdjsonParseLargeDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::large_json();

    for (auto _ : state) {
        simdjson::dom::parser parser;
        auto document = parser.parse(json);
        if (document.error()) {
            throw std::runtime_error("simdjson parse failed");
        }
        benchmark::DoNotOptimize(document.value_unsafe());
    }

    state.SetBytesProcessed(processed_bytes(json, state));
}
#endif

#ifdef HAVE_YYJSON
static void BM_YYJSONParseLargeDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::large_json();

    for (auto _ : state) {
        yyjson_doc *document =
            yyjson_read(json.data(), json.size(), YYJSON_READ_NOFLAG);
        if (document == nullptr) {
            throw std::runtime_error("yyjson parse failed");
        }
        benchmark::DoNotOptimize(yyjson_doc_get_root(document));
        yyjson_doc_free(document);
    }

    state.SetBytesProcessed(processed_bytes(json, state));
}
#endif

#ifdef HAVE_RAPIDJSON
static void BM_RapidJSONParseLargeDocument(benchmark::State &state) {
    const std::string json = benchmark_fixtures::large_json();

    for (auto _ : state) {
        rapidjson::Document document;
        document.Parse(json.c_str());
        if (document.HasParseError()) {
            throw std::runtime_error("RapidJSON parse failed");
        }
        benchmark::DoNotOptimize(document.GetType());
    }

    state.SetBytesProcessed(processed_bytes(json, state));
}
#endif

static void BM_GetPathJQ(benchmark::State &state) {
    const std::string json = benchmark_fixtures::small_json();
    jsmn_parser parser(json.c_str(), 2);
    if (parser.parse() < 0) {
        throw std::runtime_error("failed to parse small benchmark fixture");
    }

    jqpath_ptr path(jqpath_parse_string(".name[1]"));
    if (path.get() == nullptr) {
        throw std::runtime_error("failed to parse jq benchmark path");
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(parser.get_path(path.get()));
    }
}

static void BM_GetPathJSONPath(benchmark::State &state) {
    const std::string json = benchmark_fixtures::small_json();
    jsmn_parser parser(json.c_str(), 2);
    if (parser.parse() < 0) {
        throw std::runtime_error("failed to parse small benchmark fixture");
    }

    jqpath_ptr path(jsonpath_parse_string("$.name[1]"));
    if (path.get() == nullptr) {
        throw std::runtime_error("failed to parse jsonpath benchmark path");
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(parser.get_path(path.get()));
    }
}

static void BM_InsertIntoEmptyObject(benchmark::State &state) {
    const std::string json = benchmark_fixtures::empty_object_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        if (parser.parse() < 0) {
            throw std::runtime_error("failed to parse empty object fixture");
        }

        const bool inserted =
            parser.inster_kv_into_obj("\"name\"", "\"Barry\"", JSMN_STRING, 0);
        if (!inserted) {
            throw std::runtime_error("object insert benchmark failed");
        }
        benchmark::ClobberMemory();
    }
}

static void BM_InsertIntoEmptyArray(benchmark::State &state) {
    const std::string json = benchmark_fixtures::empty_array_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        if (parser.parse() < 0) {
            throw std::runtime_error("failed to parse empty array fixture");
        }

        const bool inserted =
            parser.insert_value_in_list("\"Barry\"", JSMN_STRING, 0);
        if (!inserted) {
            throw std::runtime_error("array insert benchmark failed");
        }
        benchmark::ClobberMemory();
    }
}

static void BM_UpdatePrimitiveValue(benchmark::State &state) {
    const std::string json = benchmark_fixtures::update_target_json();

    for (auto _ : state) {
        jsmn_parser parser(json.c_str(), 2);
        if (parser.parse() < 0) {
            throw std::runtime_error("failed to parse update benchmark fixture");
        }

        const int key_token = parser.find_key_in_object(0, "age");
        const bool updated = parser.update_value_for_key(key_token, "42");
        if (!updated) {
            throw std::runtime_error("update benchmark failed");
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_JSMNTokenizeOnlySmallDocument);
BENCHMARK(BM_JSMNTokenizeOnlyLargeDocument);
BENCHMARK(BM_JSMNParseAndIndexLargeDocument);
BENCHMARK(BM_RebuildPathsMediumDocument);
BENCHMARK(BM_GetPathJQ);
BENCHMARK(BM_GetPathJSONPath);
BENCHMARK(BM_InsertIntoEmptyObject);
BENCHMARK(BM_InsertIntoEmptyArray);
BENCHMARK(BM_UpdatePrimitiveValue);

#ifdef HAVE_SIMDJSON
BENCHMARK(BM_SimdjsonParseLargeDocument);
#endif

#ifdef HAVE_YYJSON
BENCHMARK(BM_YYJSONParseLargeDocument);
#endif

#ifdef HAVE_RAPIDJSON
BENCHMARK(BM_RapidJSONParseLargeDocument);
#endif

} // namespace

BENCHMARK_MAIN();
