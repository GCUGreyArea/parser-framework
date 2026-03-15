#ifndef PARSER_FRAMEWORK_UTILS_ARGS_HPP
#define PARSER_FRAMEWORK_UTILS_ARGS_HPP

#include <map>
#include <string>
#include <vector>

namespace parser_framework {
namespace utils {

class Args {
public:
    Args(int argc, const char* const* argv);

    void add_string_value(const std::string& key, const std::string& alt, const std::string& def = "");
    void add_string_values(const std::string& key, const std::string& alt, const std::vector<std::string>& def = {});
    void add_int_value(const std::string& key, const std::string& alt, int def);
    void add_key(const std::string& key, const std::string& alt);

    int get_int_value(const std::string& key) const;
    std::string get_string_value(const std::string& key) const;
    std::vector<std::string> get_string_values(const std::string& key) const;

    bool is_key_present(const std::string& key) const;
    bool is_string_present(const std::string& key) const;
    bool are_string_values_present(const std::string& key) const;

private:
    bool argument_matches(int index, const std::string& key, const std::string& alt) const;
    bool is_flag(int index) const;

    const char* const* m_argv;
    int m_argc;
    std::map<std::string, int> m_int_args;
    std::map<std::string, std::string> m_string_args;
    std::map<std::string, std::vector<std::string>> m_string_list_args;
    std::map<std::string, bool> m_bool_args;
};

} // namespace utils
} // namespace parser_framework

#endif
