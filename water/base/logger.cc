#include "logger.h"

#include <cstdio>
#include <cstring>
#include <sys/syscall.h>
#include <unistd.h>

#include <iostream>
#include <thread>

namespace water {

// 提供默认的输出函数
void DefaultOutputFunc(const std::stringstream& out) {
    fwrite(out.str().c_str(), 1, out.str().length(), stdout);
}

// 提供默认的刷新函数
void DefaultFlushFunc() {
    fflush(stdout);
}

static thread_local uint64_t last_second = 0;
static thread_local char last_time_string[32] = {0};
static thread_local pid_t thread_id = 0;
Logger::LogLevel Logger::log_level_ = DEBUG;
std::function<void(const std::stringstream&)> Logger::output_func_ = DefaultOutputFunc;
std::function<void()> Logger::flush_func_ = DefaultFlushFunc;

void Logger::FormatTime() {
    uint64_t now = date_.get_micro_seconds_since_epoch();
    uint64_t micro_sec = now % 1000000;
    now = now / 1000000;
    if (now != last_second) {
        last_second = now;
        strncpy(last_time_string, date_.ToFormattedString(false).c_str(), sizeof(last_time_string));
    }
    log_stream_ << last_time_string;
    char tmp[32];
    sprintf(tmp, "%06ld UTC ", micro_sec);
    log_stream_ << tmp;
    if (thread_id == 0) {
        thread_id = static_cast<pid_t>(::syscall(SYS_gettid));
    }
    log_stream_ << thread_id;
}

static const char* kLogLevelStr[Logger::LogLevel::NUM_LOG_LEVELS] = {
    " TRACE ",
    " DEBUG ",
    " INFO ",
    " WARN ",
    " ERROR ",
    " FATAL ",
};

Logger::Logger(SourceFile file, int line)
    : source_file_(file),
    file_line_(line),
    level_(INFO) {
    FormatTime();
    log_stream_ << kLogLevelStr[level_];
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : source_file_(file),
    file_line_(line),
    level_(level) {
    FormatTime();
    log_stream_ << kLogLevelStr[level_];
}

Logger::Logger(SourceFile file, int line, bool is_sys_err)
    : source_file_(file),
    file_line_(line),
    level_(FATAL) {
    FormatTime();
    if (errno != 0) {
        log_stream_ << kLogLevelStr[level_];
        log_stream_ << strerror(errno) << " (errno = " << errno << ") "; 
    }
}


Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
    : source_file_(file),
    file_line_(line),
    level_(level) {
    FormatTime();
    log_stream_ << kLogLevelStr[level_] << "[" << func << "] ";
}

// 在析构的时候，将日志进行格式化，然后调用 output_func_
Logger::~Logger() {
    log_stream_ << " - " << source_file_.data_ << ":" << file_line_ << std::endl;
    Logger::output_func_(log_stream_);
    if (level_ >= LogLevel::ERROR) {
        flush_func_();
    }
}

}   // namespace water
