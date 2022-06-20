#ifndef __WATER_NET_MSGBUFFER_H__
#define __WATER_NET_MSGBUFFER_H__

#include <sys/types.h>

#include <vector>
#include <string>

#define BUFFER_DEF_LEN 2048

namespace water {

class MsgBuffer {
 public:
    MsgBuffer(size_t len = BUFFER_DEF_LEN);

    // 使用默认的拷贝构造函数、赋值函数和析构函数
    
    /**
     * 获取可读区域的起始地址
     */
    const char* Peek() const { return Begin() + read_index_; }
    const uint8_t PeekInt8() const;
    const uint16_t PeekInt16() const;
    const uint32_t PeekInt32() const;
    const uint64_t PeekInt64() const;
    //read
    std::string Read(uint64_t len);
    uint8_t ReadInt8();
    uint16_t ReadInt16();
    uint32_t ReadInt32();
    uint64_t ReadInt64();
    void Swap(MsgBuffer& buf);

    /**
    * 可取数据的大小
    */
    const size_t ReadableBytes() const { return write_index_ - read_index_; }

    /**
     * 可写入数据的空间大小
     */
    const size_t WriteableBytes() const { return buffer_.size() - write_index_; }

    void Append(const MsgBuffer& buf);

    /**
     * C++ 风格
     * 添加字符串数据到可读区域后面。
     */
    void Append(const std::string& buf);

    /**
     * C 风格
     * 添加 len 大小的数据到可读区域后面。
     */
    void Append(const char *buf, size_t len);

    void AppendInt8(const uint8_t b);
    void AppendInt16(const uint16_t s);
    void AppendInt32(const uint32_t i);
    void AppendInt64(const uint64_t l);

    //add in front
    void AddInFront(const char *buf,size_t len);
    void AddInFrontInt8(const int8_t b);
    void AddInFrontInt16(const int16_t s);
    void AddInFrontInt32(const int32_t i);
    void AddInFrontInt64(const int64_t l);

    /**
     * 重置缓冲区
     */
    void RetrieveAll();

    /**
     * 缓冲区可读区容量减少 len
     * 当 len > ReadableBytes()，即大于可读区域大小是，将缓冲区重置为初始状态。
     */
    void Retrieve(size_t len);

    /**
     * 从 fd 读取数据到缓冲区。
     */
    size_t ReadFd(int fd, int *ret_errno);

 private:
    size_t read_index_;
    size_t write_index_;
    std::vector<char> buffer_;

     /**
     * 获取整个缓冲区的首地址
     */
    const char* Begin() const { return &buffer_[0]; }
    char* Begin() { return &buffer_[0]; }

    /**
     * 确保可写区域能容纳得下 len 大小的数据。如果容纳不小，则将 buffer_ 扩容或者调整。
     */
    void EnsurceWriteableBytes(size_t len);
};

}   // namespace water

#endif // __WATER_NET_MSGBUFFER_H__
