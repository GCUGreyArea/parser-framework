#include "parser_framework/utils/Args.hpp"

#include <cstdlib>

namespace parser_framework {
namespace utils {

Args::Args(int argc, const char* const* argv)
    : m_argv(argv)
    , m_argc(argc) {}

bool Args::argument_matches(int index, const std::string& key, const std::string& alt) const {
    if (index < 0 || index >= m_argc) {
        return false;
    }

    const std::string value = m_argv[index];
    return value == key || value == alt;
}

bool Args::is_flag(int index) const {
    if (index < 0 || index >= m_argc) {
        return false;
    }

    const std::string value = m_argv[index];
    return !value.empty() && value.front() == '-';
}

void Args::add_key(const std::string& key, const std::string& alt) {
    bool present = false;
    for (int index = 1; index < m_argc; ++index) {
        if (argument_matches(index, key, alt)) {
            present = true;
            break;
        }
    }

    m_bool_args[key] = present;
}

void Args::add_string_value(const std::string& key, const std::string& alt, const std::string& def) {
    std::string value = def;

    for (int index = 1; index < m_argc; ++index) {
        if (!argument_matches(index, key, alt)) {
            continue;
        }

        const int next_index = index + 1;
        if (next_index < m_argc && !is_flag(next_index)) {
            value = m_argv[next_index];
        }
        break;
    }

    m_string_args[key] = value;
}

void Args::add_string_values(const std::string& key, const std::string& alt, const std::vector<std::string>& def) {
    std::vector<std::string> values;

    for (int index = 1; index < m_argc; ++index) {
        if (!argument_matches(index, key, alt)) {
            continue;
        }

        int value_index = index + 1;
        while (value_index < m_argc && !is_flag(value_index)) {
            values.emplace_back(m_argv[value_index]);
            ++value_index;
        }

        index = value_index - 1;
    }

    if (values.empty()) {
        values = def;
    }

    m_string_list_args[key] = values;
}

void Args::add_int_value(const std::string& key, const std::string& alt, int def) {
    int value = def;

    for (int index = 1; index < m_argc; ++index) {
        if (!argument_matches(index, key, alt)) {
            continue;
        }

        const int next_index = index + 1;
        if (next_index < m_argc && !is_flag(next_index)) {
            value = std::atoi(m_argv[next_index]);
        }
        break;
    }

    m_int_args[key] = value;
}

int Args::get_int_value(const std::string& key) const {
    const auto it = m_int_args.find(key);
    if (it == m_int_args.end()) {
        return -1;
    }

    return it->second;
}

std::string Args::get_string_value(const std::string& key) const {
    const auto it = m_string_args.find(key);
    if (it == m_string_args.end()) {
        return "";
    }

    return it->second;
}

std::vector<std::string> Args::get_string_values(const std::string& key) const {
    const auto it = m_string_list_args.find(key);
    if (it == m_string_list_args.end()) {
        return {};
    }

    return it->second;
}

bool Args::is_key_present(const std::string& key) const {
    const auto it = m_bool_args.find(key);
    if (it == m_bool_args.end()) {
        return false;
    }

    return it->second;
}

bool Args::is_string_present(const std::string& key) const {
    return !get_string_value(key).empty();
}

bool Args::are_string_values_present(const std::string& key) const {
    return !get_string_values(key).empty();
}

} // namespace utils
} // namespace parser_framework
