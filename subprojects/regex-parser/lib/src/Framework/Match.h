#ifndef MATCH_H
#define MATCH_H

#include <cstddef>
#include <vector>
#include <string>

class Pattern;

class Match {
public:
    Match(std::string str)
        : mMessage(str)
        , mOffset(0) {}

    // void add_token(size_t id) {mTokenIDs.push_back(id);}
    void moveTo(size_t offset) {mOffset = offset;}

    const char * data() {return mMessage.data() + mOffset;}
    const char * raw() {return mMessage.data();}
    const char * message() {return mMessage.data();}

    size_t offset() {return mOffset;}
    void offset(size_t off) {mOffset=off;}
    void reset(std::string msg) {
        mMessage = msg;
        mOffset = 0;
    }

    void setMatch(Pattern* ptn);
    Pattern* getMathcedPattern() {return mPtn;}

    void reset() {
        mOffset = 0;
        mPtn=nullptr;
        mType = NONE;
    }

    enum Type {
        SEQUENTIAL,
        RANDOM,
        NONE
    };


    Type type() {return mType;}
    void set_match_type(Type type) {mType=type;}
    size_t message_length() {return mMessage.length();}
    size_t current_offset() {return mOffset;}
    bool end_of_message() {return mMessage.length() <= mOffset;}

private:
    std::string mMessage; //! Current message to be matched
    size_t mOffset;       //! Current offset of the last match
    Pattern* mPtn;        //! Matching pattern
    Type mType;
};

#endif//MATCH_H