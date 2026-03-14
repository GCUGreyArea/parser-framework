#ifndef ANCHOR_PATTERN_H
#define ANCHOR_PATTERN_H
#include <string>
#include <vector>

#include "Regex.h"
#include "Token.h"

class AnchorPattern {
public:
    AnchorPattern(std::string name, std::string id);
    AnchorPattern(std::string name, std::string id, std::string ptn);
    AnchorPattern(std::string name, std::string id, std::vector<std::string> anchors);
    AnchorPattern(std::string name, std::string id, std::vector<Regex> anchors);

    void addRegex(std::string ptn);

    bool match(Match& m);
    void validate();

    void reset();

    std::string get_regex_pattern();

private:
    std::string mName;
    std::string mId;
    std::vector<Regex> mAnchors;
    bool mMatched;
};

#endif//ANCHOR_PATTERN_H