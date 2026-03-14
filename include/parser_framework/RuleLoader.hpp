#ifndef PARSER_FRAMEWORK_RULE_LOADER_HPP
#define PARSER_FRAMEWORK_RULE_LOADER_HPP

#include <string>
#include <vector>

#include "parser_framework/ParserFramework.hpp"

namespace parser_framework {

namespace RuleLoader {

std::vector<MessageRule> load_rules(const std::string& path);

} // namespace RuleLoader

} // namespace parser_framework

#endif
