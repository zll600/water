#include "msg_buffer.h"

#include <cstring>
#include <cerrno>
#include <cassert>
#include <sys/uio.h>
#include <arpa/inet.h>

#include "funcs.h"

namespace water {

#define BUF_OFFSET 8

MsgBuffer::MsgBuffer(size_t len)
    : read_index_(BUF_OFFSET),
    write_index_(read_index_),
    buffer_(len + read_index_) {
}

void MsgBuffer::EnsurceWriteableBytes(size_t len) {
    if (WriteableBytes() >= len) {
        return;
    }

    if (read_index_ + WriteableBytes() >= (len + BUF_OFFSET)) {
        std::copy(Begin() + read_index_, Begin() + write_index_, Begin() + BUF_OFFSET);
        write_index_ = BUF_OFFSET + (write_index_ - read_index_);
        read_index_ = BUF_OFFSET;
        return;
    }

    // create new buffer
    size_t new_len;
    if (buffer_.size() * 2 > (BUF_OFFSET + ReadableBytes() + len)) {
        new_len = buffer_.size() * 2;
    } else {
        new_len = BUF_OFFSET + ReadableBytes() + len;
    }

    MsgBuffer new_buf(new_len);
    new_buf.Append(*this);
    Swap(new_buf);
}

void MsgBuffer::Swap(MsgBuffer& buf) {
    buffer_.swap(buf.buffer_);
    std::swap(read_index_, buf.read_index_);
    std::swap(write_index_, buf.write_index_);
}

void MsgBuffer::Append(const MsgBuffer& buf) {
    EnsurceWriteableBytes(buf.ReadableBytes());
    memcpy(&buffer_[write_index_], buf.Peek(), buf.ReadableBytes());
    write_index_ += buf.ReadableBytes();
}

void MsgBuffer::Append(const std::string& buf) {
    EnsurceWriteableBytes(buf.length());
    Append(buf.c_str(), buf.length());
}

void MsgBuffer::Append(const char *buf, size_t len) {
    EnsurceWriteableBytes(len);
    memcpy(&buffer_[write_index_], buf, len);
    write_index_ += len;
}

void MsgBuffer::Retrieve(size_t len) {
    if (len >= ReadableBytes()) {
        RetrieveAll();
        return;
    }
    read_index_ += len;
}

void MsgBuffer::RetrieveAll() {
    write_index_ = read_index_ = BUF_OFFSET;
}

size_t MsgBuffer::ReadFd(int fd, int *ret_errno) {
    char ext_buffer[65536];
    struct iovec vec[2];
    size_t writeable = WriteableBytes();
    vec[0].iov_base = Begin() + write_index_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = ext_buffer;
    vec[1].iov_len = sizeof(ext_buffer);
    const int iovcnt = (writeable < sizeof(ext_buffer) ? 2 : 1);
    ssize_t n  = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *ret_errno = errno;
    } else if (static_cast<size_t>(n) <= writeable) {
        write_index_ += n;
    } else {
        write_index_ = buffer_.size();
        Append(ext_buffer, n - writeable);
    }
    return n;
}

const uint8_t MsgBuffer::PeekInt8() const {
    assert(ReadableBytes() >= 0);
    return *(static_cast<const uint8_t*>((void*)Peek()));
}

const uint16_t MsgBuffer::PeekInt16() const {
    assert(ReadableBytes() >= 2);
    uint16_t rs = *(static_cast<const uint16_t*>((void*)Peek()));
    return ntohs(rs);
}

const uint32_t MsgBuffer::PeekInt32() const {
    assert(ReadableBytes() >= 4);
    uint32_t rl = *(static_cast<const uint32_t*>((void*)Peek()));
    return ntohl(rl);
}

const uint64_t MsgBuffer::PeekInt64() const {
    assert(ReadableBytes() >= 4);
    uint64_t rll = *(static_cast<const uint64_t*>((void*)Peek()));
    return ntoh64(rll);
}

//read
std::string MsgBuffer::Read(uint64_t len) {
    if (len > ReadableBytes()) {
        len = ReadableBytes();
    }
    std::string ret(Peek(), len);
    Retrieve(len);
    return ret;
}

uint8_t MsgBuffer::ReadInt8() {
    uint8_t ret = PeekInt8();
    Retrieve(1);
    return ret;
}

uint16_t MsgBuffer::ReadInt16() {
    uint16_t ret = PeekInt16();
    Retrieve(2);
    return ret;
}

uint32_t MsgBuffer::ReadInt32() {
    uint32_t ret = PeekInt32();
    Retrieve(4);
    return ret;
}

uint64_t MsgBuffer::ReadInt64() {
    uint64_t ret = PeekInt64();
    Retrieve(8);
    return ret;
}   

void MsgBuffer::AppendInt8(const uint8_t b) {
    Append(static_cast<const char*>((void *)&b),1);
}

void MsgBuffer::AppendInt16(const uint16_t s) {
    uint16_t ss = htons(s);
    Append(static_cast<const char*>((void*)&ss), 2);
}

void MsgBuffer::AppendInt32(const uint32_t i) {
    uint32_t ii = htonl(i);
    Append(static_cast<const char*>((void*)&ii), 4);
}

void MsgBuffer::AppendInt64(const uint64_t l) {
    uint64_t ll = hton64(l);
    Append(static_cast<const char*>((void*)&ll), 8);
}

void MsgBuffer::AddInFront(const char *buf, size_t len) {
    if (read_index_ >= len) {
        memcpy(Begin() + read_index_ - len, buf, len);
        read_index_ -= len;
        return;
    }
    size_t new_len;
    if (len + ReadableBytes() < BUFFER_DEF_LEN) {
        new_len = BUFFER_DEF_LEN;
    } else {
        new_len = len + ReadableBytes();
    }

    MsgBuffer new_buf(new_len);
    new_buf.Append(buf, len);
    new_buf.Append(*this);
    Swap(new_buf);
}

void MsgBuffer::AddInFrontInt8(const int8_t b) {
    AddInFront(static_cast<const char*>((void*)&b), 1);
}
void MsgBuffer::AddInFrontInt16(const int16_t s) {
    uint16_t ss = htons(s);
    AddInFront(static_cast<const char*>((void*)&ss), 2);
}
void MsgBuffer::AddInFrontInt32(const int32_t i) {
    uint16_t ii = htonl(i);
    AddInFront(static_cast<const char*>((void*)&ii), 4);
}
void MsgBuffer::AddInFrontInt64(const int64_t l) {
    uint64_t ll = hton64(l);
    AddInFront(static_cast<const char*>((void*)&ll), 4);
}

}   // namespace water
