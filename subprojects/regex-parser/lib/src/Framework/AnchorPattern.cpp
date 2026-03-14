#include <Framework/AnchorPattern.h>
#include <Exception/Exceptions.h>

AnchorPattern::AnchorPattern(std::string name, std::string id)
    : mName(name)
    , mId(id)
    , mMatched(false) {}

AnchorPattern::AnchorPattern(std::string name, std::string id, std::string ptn)
    : mName(name)
    , mId(id)
    , mMatched(false)
{
    Regex rx(ptn);

    mAnchors.push_back(rx);
}


AnchorPattern::AnchorPattern(std::string name, std::string id, std::vector<std::string> anchors)
    : mName(name)
    , mId(id)
{
    for(auto& a : anchors) {
        Regex rx(a);
        mAnchors.push_back(rx);
    }
}

AnchorPattern::AnchorPattern(std::string name, std::string id, std::vector<Regex> anchors)
    : mName(name)
    , mAnchors(anchors) {
}

void AnchorPattern::addRegex(std::string ptn) {
    Regex rx(ptn);

    mAnchors.push_back(rx);
}

bool AnchorPattern::match(Match& m) {
    if(mMatched)
        return true;

    std::string msg = m.data();
    Match anchor(m);

    for(auto& a : mAnchors) {
        if(!a.match(m)) {
            m.reset();
            return false;
        }
    }

    mMatched = true;
    return true;
}

void AnchorPattern::validate() {
    if(mId == "")
        throw Exception::NoId(mName);

    if(mName == "")
        throw Exception::NoName(mId);

    for(auto& r : mAnchors)
        r.validate();
}

void AnchorPattern::reset() {
    mMatched=false;
}

std::string AnchorPattern::get_regex_pattern() {
    // TODO: This will require that the anchors are all sorted!
    std::string rx{""};
    std::string last{""};
    std::string next{""};

    size_t size = mAnchors.size();
    for(size_t idx=0;idx<size;idx++) {
        auto& a = mAnchors.at(idx);
        next = a.get_regex_string();

        if(next != last) {
            rx += next;
            last = next;
            if(idx+1 < size)
                rx+= ".*?";
        }
    }

    return rx;
}