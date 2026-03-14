#ifndef YAML_H
#define YAML_H

#include <iostream>
#include <fstream>
#include <memory>

#include <yaml-cpp/yaml.h>

#include <Framework/Rule.h>
// #include <Framework/BaseProperty.h>

class YamlFile {
public:
    YamlFile(std::string name)
        : mFileName(name) {}

    void readFile();
    std::unique_ptr<Rule> getRule() {return std::move(mRule);}

private:

    struct RuleDesc {
        struct PropDesc {
            std::string name;
            std::string delim;
            std::vector<std::string> tokens;
            std::vector<std::string> expressions;
        };

        struct PatternDesc {
            std::string id;
            std::string name;
            size_t precedence;
            std::string type;
            std::string extends;
            std::string anchor;
            std::vector<std::string> tokens;
            std::string match_type;
            std::map<std::string,std::string> asserts;     //! token to asserted value map
            std::vector<PropDesc> properties;
            std::vector<std::shared_ptr<ConditionalAssignment>> dynamic;
            std::vector<PatternExample> examples;
        };

        struct TokenDesc {
            std::string name;
            std::string regex;
            std::string type;
            size_t idx;
        };

        struct AnchorDesc {
            std::string name;
            std::string id;
            std::vector<std::string> regex;
        };

        std::map<std::string, PatternDesc> patterns;    //! map of pattern desc by id
        std::map<std::string, TokenDesc> tokens;        //! map of token dec by name
        std::map<std::string, AnchorDesc> anchors;      //! map of anchors by name
        std::map<std::string,std::shared_ptr<Token>> tk_obj_map; //! Token objects by name
        std::vector<std::shared_ptr<Token>> tk_obj_list;
        std::string id;     //! The name of the rule
        std::string name;   //! the id of the rule
    };

    std::unique_ptr<AnchorPattern> build_anchor(std::string id, RuleDesc& rdp);
    void build_pattern_tokens(RuleDesc::PatternDesc& p, RuleDesc& rd, std::vector<std::shared_ptr<Token>>& tokens);
    void build_pattern_properties(RuleDesc::PatternDesc& p, RuleDesc &rd, std::vector<Property> &properties);
    std::shared_ptr<Pattern> build_pattern(std::string id, RuleDesc& rdp);
    void sort_patterns(std::vector<Pattern>& patterns);
    std::shared_ptr<ConditionalAssignment> parse_expression(std::string dynamic, RuleDesc &rd);
    void parse_examples(const YAML::Node& pattern, RuleDesc::PatternDesc& ptd);

    YAML::Node mFileNode;
    std::string mFileName;
    std::unique_ptr<Rule> mRule;
};

#endif//YAML_H
