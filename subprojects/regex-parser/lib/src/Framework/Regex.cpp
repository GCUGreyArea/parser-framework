#include "Regex.h"
#include "Exception/Exceptions.h"

Regex::Regex(std::string rxstr)
    : mRxStr(rxstr) {
        RE2 rx(mRxStr);
        if(!rx.ok())
            throw Exception::BadRegex(rxstr + " : " + rx.error_arg());

        if(rx.NumberOfCapturingGroups() > 1)
            throw Exception::TooManyCaptureGroups(rxstr);
}

Regex::Regex(const Regex& r)
    : mRxStr(r.mRxStr)
    , mResult(r.mResult) {}

/**
 * @brief Match the regex and increase the offset to the 1 + last match
 * we do this regardless of match type due to asserts that can retry a pattern
 * if there is a match that does not pass the assert, but the regex has not yet
 * reached the end of the message
 *
 * @param m
 * @return true
 * @return false
 */
bool Regex::match(Match& m) {
    RE2 rx = RE2(mRxStr);
    re2::StringPiece res;

    const char * data = nullptr;
    if(m.type() == Match::Type::RANDOM)
        data = m.raw();
    else if(m.type() == Match::Type::SEQUENTIAL)
        data = m.data();

    re2::StringPiece str(data);
    if(RE2::PartialMatch(str, rx, &res)) {
        mResult = res;
        m.offset(mResult.end() - m.raw());

        return true;
    }

    return false;
}

bool Regex::match(Match& m, std::string assert) {
    std::string sub = "(";
    sub.append(assert);
    sub.append(")");

    RE2 rxsub = RE2(sub);
    RE2 re("\\(.+\\)");
    std::string newrx = mRxStr;

    RE2::GlobalReplace(&newrx,re,sub);
    if(newrx.length() == 0) {
        throw Exception::BadRegex(sub);
    }

    RE2 rx = RE2(newrx);
    if(!rx.ok())
        throw Exception::BadRegex(sub);

    re2::StringPiece res;

    const char * data = nullptr;
    if(m.type() == Match::Type::RANDOM)
        data = m.raw();
    else if(m.type() == Match::Type::SEQUENTIAL)
        data = m.data();

    re2::StringPiece str(data);
    if(RE2::PartialMatch(str, rx, &res)) {
        mResult = res;
        m.offset(mResult.end() - m.raw());

        return true;
    }

    return false;
}


void Regex::validate() {
    if(mRxStr == "")
        throw Exception::NoRegexPattern();
}
