#ifndef RULE_VALIDATOR_H
#define RULE_VALIDATOR_H

#include <memory>
#include <vector>

class Rule;

namespace RuleValidation {

void validate_examples(std::vector<std::unique_ptr<Rule>>& rules);

}

#endif//RULE_VALIDATOR_H
