#ifndef SPAN_H
#define SPAN_H

// For size_t
#include <cstdlib>

class Span {
public:
    Span(size_t start, size_t len);
    Span(Span& span);

    bool operator==(const Span &other) const;
    bool operator!=(const Span &other) const;
    bool encapsulates(const Span& other) const;

private:
    size_t mStart;
    size_t mLength;
};

inline Span::Span(size_t start, size_t len)
    : mStart(start)
    , mLength(len) {}

inline Span::Span(Span& span)
    : mStart(span.mStart)
    , mLength(span.mLength) {}

inline bool Span::operator==(const Span &other) const {
    return mStart == other.mStart
        && mLength == other.mLength;
}

inline bool Span::operator!=(const Span &other) const {
    return !(*this == other);
}

/**
 * @brief Dose this span encapsulate the other span
 *
 * @param other
 * @return true
 * @return false
 */
inline bool Span::encapsulates(const Span& other) const {
    if(mStart <= other.mStart && mLength >= other.mLength)
        return true;
    return false;
}

#endif//SPAN_H