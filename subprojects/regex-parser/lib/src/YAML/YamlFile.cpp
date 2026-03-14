#include "YamlFile.h"

#include "Framework/Pattern.h"
#include "Framework/Token.h"
#include "Framework/AnchorPattern.h"
#include "Framework/Regex.h"
#include "Framework/Pattern.h"
#include "DynamicProperty/Compiler.h"
#include "Exception/Exceptions.h"
#include "Utils/Utils.h"

namespace {

std::string node_to_string(const YAML::Node& node)
{
    if (!node || !node.IsScalar())
        throw Exception::General("Expected scalar value in example validation block");

    return node.Scalar();
}

} // namespace

void YamlFile::readFile()
{
    mFileNode = YAML::LoadFile(mFileName);

    RuleDesc rd = {};
    rd.id = mFileNode["id"].as<std::string>();
    rd.name = mFileNode["name"].as<std::string>();

    // Iterate through the tokens first so that they are available
    const YAML::Node &tokens = mFileNode["tokens"];
    for (YAML::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        const YAML::Node &token = *it;
        RuleDesc::TokenDesc td;

        td.name = token["name"].as<std::string>();
        td.regex = token["regex"].as<std::string>();
        td.type = token["type"].as<std::string>();

        rd.tokens.emplace(td.name, td);
    }

    const YAML::Node &p = mFileNode["patterns"];

    // Iterate through all the patterns
    for (YAML::const_iterator it = p.begin(); it != p.end(); ++it)
    {
        RuleDesc::PatternDesc ptd = {};
        const YAML::Node &pattern = *it;

        ptd.id = pattern["id"].as<std::string>();
        ptd.name = pattern["name"].as<std::string>();
        ptd.precedence = pattern["precedence"].as<size_t>();
        ptd.type = pattern["type"].as<std::string>();
        ptd.match_type = pattern["match"].as<std::string>();

        if (ptd.type == "root")
        {
            ptd.anchor = pattern["anchor"].as<std::string>();
        }
        else if (ptd.type == "extends")
        {
            ptd.extends = pattern["extends"].as<std::string>();
        }
        else
        {
            throw Exception::UnknownTypeValue(ptd.type);
        }

        std::vector<std::string> tokens_list{};
        std::vector<RuleDesc::PropDesc> prop_list {};

        if (ptd.type == "extends")
        {
            RuleDesc::PatternDesc ptd_next = ptd;

            // Get all the derived tokens first
            while (ptd_next.type != "root")
            {
                auto pit = rd.patterns.find(ptd_next.extends);
                if (pit != rd.patterns.end())
                {
                    ptd_next = pit->second;
                    for (auto t : ptd_next.tokens)
                    {
                        tokens_list.push_back(t);
                    }

                    for(auto p : ptd_next.properties) {
                        prop_list.push_back(p);
                    }
                }
                else
                {
                    std::cout << "Couldn't find pattern for " << ptd_next.id << std::endl;
                    throw Exception::PatternNotFound(ptd_next.extends);
                }
            }

            // Set the anchor for this pattern to the root anchor pattern
            ptd.anchor = ptd_next.anchor;
        }

        // Now get all the declared tokens
        const YAML::Node &tokens = pattern["tokens"];

        // This is a list of tokens that the pattern declares
        for (YAML::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
        {
            std::string token = it->as<std::string>();
            tokens_list.push_back(token);
        }

        ptd.tokens.assign(tokens_list.begin(), tokens_list.end());
        ptd.properties.assign(prop_list.begin(), prop_list.end());

        YAML::Node asserts = pattern["assert"];
        for (YAML::iterator it = asserts.begin(); it != asserts.end(); ++it)
        {
            const YAML::Node as = *it;

            std::string token = as["token"].as<std::string>();
            std::string assert = as["value"].as<std::string>();

            ptd.asserts.emplace(token, assert);
        }

        YAML::Node properties = pattern["properties"];
        for (YAML::iterator it = properties.begin(); it != properties.end(); ++it)
        {
            const YAML::Node &pr = *it;
            RuleDesc::PropDesc pd;

            try
            {
                pd.name = pr["name"].as<std::string>();
                pd.delim = pr["deliminator"].as<std::string>();

                const YAML::Node &tokens = pr["tokens"];
                for (YAML::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
                {
                    std::string token = it->as<std::string>();
                    pd.tokens.push_back(token);
                }
            }
            catch (std::exception& e)
            {
                // This must be a dynamic property
                pd.name = "dynamic";
                auto expression = pr["dynamic"].as<std::string>();
                pd.expressions.push_back(expression);
            }

            ptd.properties.push_back(pd);
        }

        rd.patterns.emplace(ptd.id, ptd);

        parse_examples(pattern, rd.patterns.at(ptd.id));
    }

    // Iterate through the anchors
    YAML::Node anchors = mFileNode["anchors"];
    for (YAML::const_iterator it = anchors.begin(); it != anchors.end(); ++it)
    {
        const YAML::Node &anchor = *it;
        RuleDesc::AnchorDesc ad = {};
        ad.name = anchor["name"].as<std::string>();
        ad.id = anchor["id"].as<std::string>();

        const YAML::Node &patterns = anchor["patterns"];
        for (YAML::const_iterator it = patterns.begin(); it != patterns.end(); ++it)
        {
            std::string astr = it->as<std::string>();
            ad.regex.push_back(astr);
        }

        rd.anchors.emplace(ad.id, ad);
    }

    // Create the patterns for the rule
    std::vector<Pattern> patterns{};
    for (auto &p : rd.patterns)
    {
        std::shared_ptr<Pattern> ptn = build_pattern(p.second.id, rd);
        patterns.push_back(*ptn);
    }

    // Sort the vector by pattern size then precedence
    sort_patterns(patterns);

    mRule = std::make_unique<Rule>(rd.id, rd.name, patterns, rd.tk_obj_list);
}

std::unique_ptr<AnchorPattern> YamlFile::build_anchor(std::string id, RuleDesc &rd)
{
    auto ait = rd.anchors.find(id);
    if (ait != rd.anchors.end())
    {
        std::vector<Regex> regex;
        auto an = ait->second;
        for (auto r : an.regex)
        {
            Regex rx(r);
            regex.push_back(rx);
        }
        if (regex.size() == 0)
            throw Exception::NoAnchorPatterns(an.id);

        return std::make_unique<AnchorPattern>(an.name, an.id, regex);
    }

    throw Exception::AnchorNotFound(id);
}


void YamlFile::build_pattern_properties(RuleDesc::PatternDesc& p, RuleDesc &rd, std::vector<Property> &properties)
{
    for (auto& prop : p.properties)
    {
        if (prop.tokens.size() > 0)
        {
            std::vector<std::shared_ptr<Token>> tokens;
            for (auto t : prop.tokens) {

                auto toit = rd.tk_obj_map.find(t);
                if (toit == rd.tk_obj_map.end())
                    throw Exception::TokenNotFound(t);

                tokens.push_back(toit->second);
            }

            properties.emplace_back(tokens, prop.name, prop.delim);

            continue;
        }
        else if(prop.expressions.size() > 0) {
            for(auto expression : prop.expressions) {
                auto dyn = parse_expression(expression,rd);
                if(dyn)
                    p.dynamic.push_back(dyn);
            }
        }
    }
}

void YamlFile::build_pattern_tokens(RuleDesc::PatternDesc& p, RuleDesc &rd, std::vector<std::shared_ptr<Token>> &tokens)
{
    for (auto t : p.tokens)
    {
        auto tit = rd.tokens.find(t);
        if (tit == rd.tokens.end())
            throw Exception::TokenNotFound(t);

        std::shared_ptr<Token> token = nullptr;

        auto toit = rd.tk_obj_map.find(t);
        if (toit == rd.tk_obj_map.end())
        {
            auto tk = tit->second;
            ValueType type(tk.type);

            if (type == "none")
                throw Exception::UnknownTypeValue(tk.type);

            token = std::make_shared<Token>(tk.name, tk.regex, type, tokens.size());
            rd.tk_obj_map.emplace(t, token);
            rd.tk_obj_list.push_back(token);
        }
        else
        {
            token = toit->second;
        }

        tokens.push_back(token);
    }
    if (tokens.size() == 0)
        throw Exception::PatternHasNoTokens(p.id);
}

std::shared_ptr<Pattern> YamlFile::build_pattern(std::string id, RuleDesc &rd)
{
    auto pit = rd.patterns.find(id);
    if (pit == rd.patterns.end())
        throw Exception::PatternNotFound(id);

    RuleDesc::PatternDesc ptn = pit->second;
    std::unique_ptr<AnchorPattern> aptn = build_anchor(ptn.anchor, rd);

    std::vector<std::shared_ptr<Token>> tokens = {};
    std::vector<Property> properties {};
    std::vector<size_t> token_idx = {};

    build_pattern_tokens(ptn, rd, tokens);
    build_pattern_properties(ptn, rd, properties);

    Match::Type type = Match::Type::NONE;

    if (ptn.match_type == "sequential")
        type = Match::Type::SEQUENTIAL;
    else if (ptn.match_type == "random")
        type = Match::Type::RANDOM;
    else
        throw Exception::UnknownMatchType(ptn.match_type);

    std::vector<std::string> asserts = {};

    for (auto t : ptn.tokens)
    {
        auto it = ptn.asserts.find(t);
        if (it != ptn.asserts.end())
        {
            // std::cout << "Adding [assert " << it->second << " / to pattern " << ptn.id << "]" << std::endl;
            asserts.push_back(it->second);
        }
        else
            asserts.push_back("");
    }

    // TODO: Make this into a builder?
    auto p = std::make_shared<Pattern>(ptn.name, ptn.id, *aptn, tokens, asserts, properties, ptn.precedence, type);

    for (const auto& example : ptn.examples) {
        p->add_example(example);
    }

    for(auto ds : ptn.dynamic) {
        p->add_conditional_assignment(ds);
    }
    return p;
}

/**
 * @brief Sort token list first by number of tokens, then precedence
 * The more specific pattern should be tried first!
 *
 * @param patterns
 */
void YamlFile::sort_patterns(std::vector<Pattern>& patterns)
{
    std::sort(patterns.begin(), patterns.end(), [](const Pattern& lhs, const Pattern& rhs)
              {
        if(lhs.num_tokens() != rhs.num_tokens()) {
            return lhs.num_tokens() > rhs.num_tokens();
        }

        return lhs.precedence() < rhs.precedence(); });
}

/**
 * @brief Parse and compile a dynamic property expression into an executable
 * conditional assignment using the generated lexer/parser pipeline.
 */
std::shared_ptr<ConditionalAssignment> YamlFile::parse_expression(std::string dynamic, RuleDesc &rd)
{
    return DynamicProperty::compile(dynamic, rd.tk_obj_map);
}

void YamlFile::parse_examples(const YAML::Node& pattern, RuleDesc::PatternDesc& ptd)
{
    auto parse_example_node = [&](const YAML::Node& example_node) {
        PatternExample example = {};

        if (example_node.IsScalar())
        {
            example.message = example_node.as<std::string>();
            ptd.examples.push_back(example);
            return;
        }

        if (!example_node.IsMap())
            throw Exception::General("Pattern example must be a scalar or map");

        YAML::Node message = example_node["message"];
        if (!message)
            throw Exception::General("Pattern example map must declare a message");

        example.message = message.as<std::string>();

        YAML::Node expect = example_node["expect"];
        if (expect)
        {
            YAML::Node tokens = expect["tokens"];
            if (tokens)
            {
                for (YAML::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
                {
                    example.expected_tokens.emplace(it->first.as<std::string>(), node_to_string(it->second));
                }
            }

            YAML::Node properties = expect["properties"];
            if (properties)
            {
                for (YAML::const_iterator it = properties.begin(); it != properties.end(); ++it)
                {
                    example.expected_properties.emplace(it->first.as<std::string>(), node_to_string(it->second));
                }
            }

            YAML::Node absent_properties = expect["absent_properties"];
            if (absent_properties)
            {
                for (YAML::const_iterator it = absent_properties.begin(); it != absent_properties.end(); ++it)
                {
                    example.absent_properties.push_back(it->as<std::string>());
                }
            }
        }

        ptd.examples.push_back(example);
    };

    YAML::Node example = pattern["example"];
    if (example && !example.IsNull())
    {
        parse_example_node(example);
    }

    YAML::Node examples = pattern["examples"];
    if (examples && !examples.IsNull())
    {
        if (!examples.IsSequence())
            throw Exception::General("Pattern examples must be a sequence");

        for (YAML::const_iterator it = examples.begin(); it != examples.end(); ++it)
        {
            parse_example_node(*it);
        }
    }
}
