#include "Pattern.h"
#include "Token.h"
#include "Exception/Exceptions.h"

Pattern::Pattern(std::string name, std::string id, AnchorPattern& anchor, size_t precedence)
    : mName(name)
    , mId(id)
    , mPrecedence(precedence)
    , mAnchor(anchor)
    , mMatched(false)
    , mMatchType(Match::Type::SEQUENTIAL) {}

Pattern::Pattern(std::string name, std::string id, AnchorPattern& anchor, std::vector<std::shared_ptr<Token>> tokens, std::vector<std::string> asserts, std::vector<Property> properties, size_t precedence, Match::Type type)
    : mName(name)
    , mId(id)
    , mPrecedence(precedence)
    , mAnchor(anchor)
    , mTokens(tokens)
    , mTkAsserts(asserts)
    , mMatched(false)
    , mMatchType(type)
    , mProperties(properties) {}



void Pattern::add_token(std::string name, std::string ptn, ValueType type, size_t idx) {
    std::shared_ptr<Token> tk = std::make_shared<Token>(name,ptn,type,idx);
    mTokens.push_back(tk);
}

/**
 * @brief Match a pattern to a sequence of tokens in the message
 *
 * @todo Substitute the Regex pattern for one with the asserted value to minimise
 * execution time
 * @param m
 * @return true
 * @return false
 */
bool Pattern::match(Match &m, bool anchor) {
    m.reset();
    m.set_match_type(mMatchType);

    if(anchor) {
        if (!mAnchor.match(m)){
            return false;
        }
        else {
            m.reset();
            m.set_match_type(mMatchType);
        }
    }

    size_t size = mTokens.size();
    for (size_t i = 0; i < size; i++) {
        auto &t = mTokens.at(i);
        if(mTkAsserts.size() > 0){
            auto &a = mTkAsserts.at(i);
            if (a != "") {
                if (!t->match(m, a))
                    return false;
            }
            if (!t->match(m))
                return false;
        }
        else {
            if (!t->match(m))
                return false;
        }
    }

    m.setMatch(this);
    mMatched = true;
    return true;
}


bool Pattern:: matched() {
    return mMatched;
}

void Pattern::validate() {
    if(mName == "")
        throw Exception::NoName(mId);

    if(mId == "")
        throw Exception::NoId(mName);

    mAnchor.validate();

    for(auto& t : mTokens)
        t->validate();
}

void Pattern::reset() {
    mMatched = false;
    mAnchor.reset();

    for(auto& t : mTokens)
        t->reset();
}

void Pattern::print_tokens() {
    for(auto& t : mTokens) {
        std::cout << "Token [" << t->name() << " / value " << t->as_string() << "]" << std::endl;
    }
}
