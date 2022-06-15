#ifndef __WATER_BASE_LOGSTREAM_H__
#define __WATER_BASE_LOGSTREAM_H__

#include <cstring>

#include <string>

#include "noncopyable.h"

namespace water {

namespace detail {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

// 固定大小的缓冲区
template <int SIZE>
class FixedBuffer : Noncopyable {
 public:
    FixedBuffer() : cur_(data_) {
        SetCookie(CookieStart);
    }

    ~FixedBuffer() {
        SetCookie(CookieEnd);
    }

    // 追加 buf 内容
    void Append(const char * /*restrict*/ buf, size_t len);

    // 获取底层缓冲区的指针
    const char* get_data() const { return data_; }
    // 获取未写入的内容的长度
    int Length() const { return cur_ - data_; }

    // 获取指向当前位置的指针
    char* get_cur() const { return cur_; }
    // 获取可以继续写入的空间爱大小
    int Avail() const { return static_cast<int>(End() - cur_); }
    // 移动 cur_ 指针
    void Add(size_t len) { cur_ += len; }

    // 恢复指针指向缓冲区开始
    void Reset() { cur_ = data_; }
    // 初始化缓冲区
    void Bzero() { ::bzero(data_, sizeof(data_)); }

    
    // for debug
    const char* DebugStr();
    std::string ToString() const { return std::string(data_, Length()); }
    void SetCookie(void (*cookie)()) { cookie_ = cookie; }
 private:
    void (*cookie_)();
    char data_[SIZE];
    char *cur_;

    // 获取指向缓冲区末尾的指针
    const char* End() const {
        return data_ + sizeof(data_);
    }

    static void CookieStart();
    static void CookieEnd();
};

}   // namespace water::detail

// 将内容写入到缓冲区中
class LogStream : Noncopyable {
 public:
    using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

    LogStream& operator<<(bool val) {
        buffer_.Append(val ? "1" : "0", 1);
        return *this;
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(const void*);

    LogStream& operator<<(float val) {
        *this << static_cast<double>(val);
        return *this;
    }

    LogStream& operator<<(double);
    // LogStream& operator<<(long double);

    LogStream& operator<<(char val) {
        buffer_.Append(&val, 1);
        return *this;
    }

    // LogStream& operator<<(signed char);
    // LogStream& operator<<(unsigned char);

    LogStream& operator<<(const char* str) {
        if (str) {
            buffer_.Append(str, strlen(str));
        } else {
            buffer_.Append("(null)", 6);
        }

        return *this;
    }

    LogStream& operator<<(const unsigned char* str) {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    LogStream& operator<<(const std::string& str) {
        buffer_.Append(str.c_str(), str.size());
        return *this;
    }

    void Append(const char *data, size_t len) { buffer_.Append(data, len); }
    const Buffer& get_buffer() const { return buffer_; }
    void ResetBuffer() { buffer_.Reset(); }

 private:
    Buffer buffer_;
    
    static const int kMaxNumericSize = 32;

    void StaticCheck();
    template <typename T>
    void FormatInteger(T);
};

// 日志格式
class Fmt : Noncopyable {
 public:
    template <typename T>
    Fmt(const char* buf, T val);
    
    const char* get_data() const { return data_; }
    int get_len() const { return len_; }
 private:
    char data_[32];
    int len_;
};

inline LogStream& operator<<(LogStream& log_stream, const Fmt& fmt) {
    log_stream.Append(fmt.get_data(), fmt.get_len());
    return log_stream;
}

}   // namespace water

#endif // __WATER_BASE_LOGSTREAM_H__
