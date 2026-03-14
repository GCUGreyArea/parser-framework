#ifndef RULE_LOADER_H
#define RULE_LOADER_H

#include <memory>
#include <string>
#include <vector>

class Rule;

namespace RuleLoader {

std::vector<std::unique_ptr<Rule>> load_rules(const std::string& dir);

}

#endif//RULE_LOADER_H
