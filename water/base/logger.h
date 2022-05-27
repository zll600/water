#ifndef __WATER_BASE_LOGGER_H__
#define __WATER_BASE_LOGGER_H__

#include <cstring>
#include <cstdint>

#include <string>
#include <functional>
#include <sstream>

#include "noncopyable.h"
#include "date.h"

namespace water {

class Logger : public Noncopyable {
 public:
    enum LogLevel {
        TRACE = 0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };
    
    // compile time calculation of basename of source file
    class SourceFile {
     public:
        template <int N>
        inline SourceFile(const char (&arr)[N])
            : data_(arr),
            size_(N - 1) {
            const char *slash = strchr(data_, '/');
            if (slash) {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
            
        }

        explicit SourceFile(const char *filename)
            : data_(filename) {
            const char *slash = strchr(filename, '/');
            if (slash) {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

        const char *data_;
        int size_;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, bool is_sys_err);
    Logger(SourceFile file, int line, LogLevel, const char *func);
    // 析构时将日志输出
    ~Logger();

    std::stringstream& get_log_stream() { return log_stream_; }

    static void set_output_func(std::function<void(const char*, uint64_t)> func) {
        output_func_ = func;
    }
    static void set_log_level(LogLevel level) {
        log_level_ = level;
    }
    static LogLevel get_log_level() {
        return log_level_;
    }

 protected:
    std::stringstream log_stream_;  // 流式写日志
    Date date_ = Date::Now();   // 时间
    SourceFile source_file_;    // 文件名
    int file_line_;     // 行号
    LogLevel level_;    // 日志级别
    static uint64_t last_second_;
    static std::string last_time_string_;   // 保存上次格式化后的时间
    static LogLevel log_level_;     // 日志级别
    static std::function<void(const char*, uint64_t)> output_func_; // 将日志输出的目的地

    // 格式化时间
    void FormatTime();
};

#define LOG_TRACE if (water::Logger::get_log_level() <= water::Logger::LogLevel::TRACE) \
                        water::Logger(__FILE__, __LINE__, water::Logger::LogLevel::TRACE, __func__).get_log_stream()

#define LOG_DEBUG if (water::Logger::get_log_level() <= water::Logger::LogLevel::DEBUG) \
                        water::Logger(__FILE__, __LINE__, water::Logger::LogLevel::DEBUG, __func__).get_log_stream()

#define LOG_INFO if (water::Logger::get_log_level() <= water::Logger::LogLevel::INFO) \
                        water::Logger(__FILE__, __LINE__, water::Logger::LogLevel::INFO).get_log_stream()

#define LOG_WARN water::Logger(__FILE__, __LINE__, water::Logger::LogLevel::WARN).get_log_stream()
#define LOG_ERROR water::Logger(__FILE__, __LINE__, water::Logger::LogLevel::ERROR).get_log_stream()
#define LOG_FATAL water::Logger(__FILE__, __LINE__, water::Logger::LogLevel::FATAL).get_log_stream()
#define LOG_SYSERR water::Logger(__FILE__, __LINE__, true).get_log_stream()

}   // namespace water

#endif // __WATER_BASE_LOGGER_H__
