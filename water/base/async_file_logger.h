#ifndef __WATER_BASE_ASYNCFILELOGGER_H__
#define __WATER_BASE_ASYNCFILELOGGER_H__

#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <queue>

#include "noncopyable.h"
#include "date.h"

namespace water {

using StringPtr = std::shared_ptr<std::string>;
using StringPtrQueue = std::queue<StringPtr>;

// 异步文件日志
class AsyncFileLogger : Noncopyable {
 public:
    AsyncFileLogger();
    // 析构函数中，join 日志线程
    ~AsyncFileLogger();

    // 日志输出
    void Output(const char *msg, uint64_t len);
    // 刷新函数
    void Flush();
    // 创建一个后台线程写日志
    void StartLogging();
    // 设置单个日志文件的最大大小
    void set_file_size_limit(uint64_t size_limit) { size_limit_ = size_limit; }
    // 设置文件名称
    void SetFileName(const std::string& base_name, const std::string& ext_name = ".log", const std::string& path = "./");
 protected:
    // 管理日志文件
    class LoggerFile : Noncopyable {
     public:
        LoggerFile(const std::string& file_path, const std::string& file_base_name, const std::string& file_ext_name);
        ~LoggerFile();
        
        // 将日志写入文件
        void WriteLog(const StringPtr buf);
        // 获取文件长度
        uint64_t GetLength() const;
        explicit operator bool() const { return fp_ != nullptr; }
     protected:
        FILE *fp_ = nullptr;    // 文件指针
        Date create_date_;  // 创建日期
        std::string file_full_name_;    // 文件名陈
        std::string file_path_;     // 文件路径
        std::string file_base_name_;    // 文件名
        std::string file_ext_name_; // 文件名后缀
        
        static uint64_t file_seq_;
    };

    std::mutex mut_;    // 互斥锁
    std::condition_variable cond_;  // 条件变量
    StringPtr log_buffer_ptr_;    // 日志缓冲区
    StringPtrQueue write_buffers_;
    std::unique_ptr<std::thread> thread_ptr_;   // 异步线程
    bool stop_flag_;    // 停止标志
    std::string file_path_ = "./";   // 文件路径
    std::string file_base_name_ = "water";   // 文件名
    std::string file_ext_name_ = ".log"; // 文件路径
    uint64_t size_limit_ = 20 * 1024 * 1024;    // 文件大小限制
    std::unique_ptr<LoggerFile> logger_file_ptr_;   // 日志文件指针
    uint64_t lost_counter_ = 0;

    // 将日志写入文件
    void WriteLogToFile(const StringPtr buf);
    // 异步写日志
    void LogThreadFunc();
};

}   // namespace water

#endif // __WATER_BASE_ASYNCFILELOGGER_H__
