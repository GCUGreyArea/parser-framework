#ifndef PARSER_FRAMEWORK_DYNAMIC_PROPERTY_ENGINE_HPP
#define PARSER_FRAMEWORK_DYNAMIC_PROPERTY_ENGINE_HPP

#include <map>
#include <string>
#include <vector>

#include "parser_framework/ParserFramework.hpp"

namespace parser_framework {

class DynamicPropertyEngine {
public:
    std::map<std::string, std::string> evaluate(
        const std::vector<std::string>& expressions,
        const ParseResult& result,
        std::vector<std::string>& errors) const;
};

} // namespace parser_framework

#endif
