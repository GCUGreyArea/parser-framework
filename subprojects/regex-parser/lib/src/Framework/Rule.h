#ifndef RULE_H
#define RULE_H

#include <unordered_map>

#include "Pattern.h"
#include "RootPath.h"

class Rule {
public:
    Rule(std::string id, std::string name, std::vector<Pattern> patterns, std::vector<std::shared_ptr<Token>> tokens);
    Rule(Rule& r);

    Pattern* match(Match& m, bool anchor = true);
    void reset();

    std::string id() {return mId;}
    std::string name() {return mName;}
    Pattern* matched() {return mMatched;}
    std::vector<Pattern>& patterns() {return mPatterns;}
    const std::vector<Pattern>& patterns() const {return mPatterns;}

    std::vector<std::string> get_anchor_regex();

    void describe_as_json(nlohmann::json& out);


private:
    std::string mId;                                //! The rule ID
    std::string mName;                              //! Name of the rule
    std::vector<Pattern> mPatterns;                 //! A list of patterns
    std::vector<std::shared_ptr<Token>> mTokens;    //! All the tokens available to a rule
    Pattern* mMatched;                              //! The pattern for the last match
};

#endif//RULE_H
