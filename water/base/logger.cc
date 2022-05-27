#include "logger.h"

#include <cstdio>

#include <iostream>

namespace water {

void DefaultOutputFunc(const char *msg, uint64_t length) {
    fwrite(msg, 1, length, stdout);
}

uint64_t Logger::last_second_ = 0;
std::string Logger::last_time_string_;
Logger::LogLevel Logger::log_level_ = DEBUG;
std::function<void(const char*, uint64_t)> Logger::output_func_ = DefaultOutputFunc;

void Logger::FormatTime() {
    uint64_t now = date_.get_micro_seconds_since_epoch();
    uint64_t micro_sec = now % 1000000;
    now = now / 1000000;
    if (now != last_second_) {
        last_second_ = now;
        last_time_string_ = date_.ToFormattedString(false);
    }
    log_stream_ << last_time_string_;
    char tmp[32];
    sprintf(tmp, "%06ld GMT ", micro_sec);
    log_stream_ << tmp;
}

static const char* kLogLevelStr[Logger::LogLevel::NUM_LOG_LEVELS] = {
    "TRACE ",
    "DEBUG ",
    "INFO ",
    "WARN ",
    "ERROR ",
    "FATAL ",
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

Logger::~Logger() {
    log_stream_ << " - " << source_file_.data_ << ":" << file_line_ << std::endl;
    Logger::output_func_(log_stream_.str().c_str(), log_stream_.str().length());
}

}   // namespace water
