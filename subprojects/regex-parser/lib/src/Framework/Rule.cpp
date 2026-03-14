#include "Rule.h"


Rule::Rule(std::string id, std::string name, std::vector<Pattern> patterns, std::vector<std::shared_ptr<Token>> tokens)
    : mId(id)
    , mName(name)
    , mPatterns(patterns)
    , mTokens(tokens)
    , mMatched(nullptr) {}

Rule::Rule(Rule& r)
    : mId(r.mId)
    , mName(r.mName)
    , mPatterns(r.mPatterns)
    , mTokens(r.mTokens)
    , mMatched(nullptr) {}

Pattern* Rule::match(Match& m, bool anchor) {
    reset();

    for(auto& p : mPatterns) {
        if(p.match(m,anchor)) {
            mMatched = &p;
            break;
        }

        m.reset();
    }

    return mMatched;
}


void Rule::reset() {
    for(auto& p : mPatterns)
        p.reset();
    mMatched = nullptr;
}

std::vector<std::string> Rule::get_anchor_regex() {
    std::vector<std::string> rx{};
    std::string next{""};
    std::string last{""};
    for(auto& p : mPatterns) {
        next = p.get_anchor_regex_pattern();
        if(next != last) {
            last = next;
            rx.push_back(next);
        }
    }

    return rx;
}