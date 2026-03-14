#ifndef BENCHMARK_FIXTURES_H
#define BENCHMARK_FIXTURES_H

#include <fstream>
#include <stdexcept>
#include <string>

namespace benchmark_fixtures {

inline std::string small_json() {
    return "{\"name\":[\"value 1\",\"value 2\",\"value 3\"],"
           "\"meta\":{\"active\":true,\"count\":3}}";
}

inline std::string medium_json() {
    return "{\"user\":{\"name\":\"Barry\",\"age\":12,\"tags\":[\"dev\",\"ops\"],"
           "\"stats\":{\"score\":99,\"active\":true}},"
           "\"items\":[{\"id\":1,\"value\":\"one\"},{\"id\":2,\"value\":\"two\"},"
           "{\"id\":3,\"value\":\"three\"}]}";
}

inline std::string empty_object_json() {
    return "{}";
}

inline std::string empty_array_json() {
    return "[]";
}

inline std::string update_target_json() {
    return "{\"age\":12,\"name\":\"Barry\"}";
}

inline std::string large_json() {
    static const std::string cached = []() {
        std::ifstream file("test/resources/test-data/large-file.json");
        if (!file.is_open()) {
            throw std::runtime_error("failed to open benchmark fixture JSON");
        }

        file.seekg(0, std::ios::end);
        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::string json(static_cast<size_t>(size), '\0');
        file.read(&json[0], size);
        if (!file) {
            throw std::runtime_error("failed to read benchmark fixture JSON");
        }

        return json;
    }();

    return cached;
}

} // namespace benchmark_fixtures

#endif
