#include "parser_framework/MessageLoader.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace parser_framework {
namespace MessageLoader {

namespace {

bool is_message_file(const std::filesystem::path& path) {
    const std::string extension = path.extension().string();
    return extension == ".log" || extension == ".json" || extension == ".txt";
}

std::string trim_trailing_newlines(std::string input) {
    while (!input.empty() && (input.back() == '\n' || input.back() == '\r')) {
        input.pop_back();
    }

    return input;
}

} // namespace

std::string load_message_file(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Failed to open message file [" + path + "]");
    }

    return trim_trailing_newlines(std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()));
}

std::vector<std::string> load_messages_from_path(const std::string& path) {
    namespace fs = std::filesystem;

    const fs::path input(path);
    if (!fs::exists(input)) {
        throw std::runtime_error("Message path does not exist [" + path + "]");
    }

    if (fs::is_regular_file(input)) {
        return {load_message_file(input.string())};
    }

    if (!fs::is_directory(input)) {
        throw std::runtime_error("Message path is not a file or directory [" + path + "]");
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(input)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (is_message_file(entry.path())) {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end());

    std::vector<std::string> messages;
    messages.reserve(files.size());
    for (const auto& file : files) {
        messages.push_back(load_message_file(file.string()));
    }

    return messages;
}

std::vector<std::string> load_messages_from_paths(const std::vector<std::string>& paths) {
    std::vector<std::string> messages;

    for (const auto& path : paths) {
        const std::vector<std::string> loaded = load_messages_from_path(path);
        messages.insert(messages.end(), loaded.begin(), loaded.end());
    }

    return messages;
}

} // namespace MessageLoader
} // namespace parser_framework
