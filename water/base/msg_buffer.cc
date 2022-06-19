#include "msg_buffer.h"

#include <cstring>
#include <cerrno>
#include <sys/uio.h>

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
    MsgBuffer new_buf(BUF_OFFSET + ReadableBytes() + len);
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

}   // namespace water
