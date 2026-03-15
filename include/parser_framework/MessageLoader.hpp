#ifndef PARSER_FRAMEWORK_MESSAGE_LOADER_HPP
#define PARSER_FRAMEWORK_MESSAGE_LOADER_HPP

#include <string>
#include <vector>

namespace parser_framework {

namespace MessageLoader {

std::string load_message_file(const std::string& path);
std::vector<std::string> load_messages_from_paths(const std::vector<std::string>& paths);
std::vector<std::string> load_messages_from_path(const std::string& path);

} // namespace MessageLoader

} // namespace parser_framework

#endif
