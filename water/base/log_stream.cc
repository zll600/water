#include "log_stream.h"

#include <algorithm>
#include <limits>

namespace water {

namespace detail {

const char kDigits[] = "9876543210123456789";
const char *kZero = kDigits + 9;

const char kDigitsHex[] = "0123456789ABCDEF";

// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
size_t Convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = kZero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

size_t ConvertHex(char buf[], uintptr_t value) {
    uintptr_t i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = kDigitsHex[lsd];
    } while (i != 0);

    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template <int SIZE>
void FixedBuffer<SIZE>::Append(const char * /* restrict */ buf, size_t len) {
    if (static_cast<size_t>(Avail()) > len) {
        memcpy(cur_, buf, len);
        cur_ += len;
    }
}

template <int SIZE>
const char* FixedBuffer<SIZE>::DebugStr() {
    *cur_ = '\0';
    return data_;
}

template <int SIZE>
void FixedBuffer<SIZE>::CookieStart() {
}

template <int SIZE>
void FixedBuffer<SIZE>::CookieEnd() {
}

}   // namespace water::detail

void LogStream::StaticCheck() {
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10,"");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10,"");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10,"");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10,"");
}

template <typename T>
void LogStream::FormatInteger(T val) {
    if (buffer_.Avail() >= kMaxNumericSize) {
        size_t len = detail::Convert(buffer_.get_cur(), val);
        buffer_.Add(len);
    }
}

LogStream& LogStream::operator<<(short val) {
    *this << static_cast<int>(val);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short val) {
    *this << static_cast<unsigned int>(val);
    return *this;
}

LogStream& LogStream::operator<<(int val) {
    FormatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int val) {
    FormatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(long val) {
    FormatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long val) {
    FormatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(long long val) {
    FormatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long val) {
    FormatInteger(val);
    return *this;
}

LogStream& LogStream::operator<<(const void* ptr) {
    uintptr_t val = reinterpret_cast<uintptr_t>(ptr);
    if (buffer_.Avail() >= kMaxNumericSize) {
        char *buf = buffer_.get_cur();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = detail::ConvertHex(buf + 2, val);
        buffer_.Add(len);
    }
    return *this;    
}

LogStream& LogStream::operator<<(const double val) {
    if (buffer_.Avail() >= kMaxNumericSize) {
        size_t len = snprintf(buffer_.get_cur(), kMaxNumericSize, "%.12g", val);
        buffer_.Add(len);
    }
    return *this;
}

// Explicit instantiations

template<>
Fmt::Fmt(const char* fmt, char);
template<> 
Fmt::Fmt(const char* fmt, short);
template<> 
Fmt::Fmt(const char* fmt, unsigned short);
template<> 
Fmt::Fmt(const char* fmt, int);
template<> 
Fmt::Fmt(const char* fmt, unsigned int);
template<> 
Fmt::Fmt(const char* fmt, long);
template<> 
Fmt::Fmt(const char* fmt, unsigned long);
template<> 
Fmt::Fmt(const char* fmt, long long);
template<> 
Fmt::Fmt(const char* fmt, unsigned long long);
template<> 
Fmt::Fmt(const char* fmt, float);
template<> 
Fmt::Fmt(const char* fmt, double);

}   // namespace water
