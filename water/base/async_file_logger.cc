#include "async_file_logger.h"

#include <cstdio>
#include <cstring>

#include <iostream>
#include <sstream>
#include <functional>
#include <chrono>

#define LOG_FLUSH_TIMEOUT 1
#define MEM_BUFFER_SIZE 4096

namespace water {

AsyncFileLogger::LoggerFile::LoggerFile(const std::string& file_path, const std::string& file_base_name,
        const std::string& file_ext_name)
    : create_date_(Date::Now()),
    file_path_(file_path),
    file_base_name_(file_base_name),
    file_ext_name_(file_ext_name) {
    file_full_name_ = file_path_ + file_base_name_ + file_ext_name_;
    fp_ = fopen(file_full_name_.c_str(), "a");
    if (fp_ == nullptr) {
        std::cout << strerror(errno) << std::endl;
    }
}

AsyncFileLogger::LoggerFile::~LoggerFile() {
    if (fp_) {
        fclose(fp_);
        
        // 将文件重命名
        std::string new_name = file_path_ + file_base_name_ + "." + create_date_.ToCustomedFormattedString("%y%m%d-%H%M%S") + file_ext_name_;
        rename(file_full_name_.c_str(), new_name.c_str());
    }   
}
        
void AsyncFileLogger::LoggerFile::WriteLog(const std::string& buf) {
    if (fp_) {
        fwrite(buf.c_str(),1,buf.length(),fp_);
    }
}

uint64_t AsyncFileLogger::LoggerFile::GetLength() const {
    if (fp_) {
        return ftell(fp_);
    }
    return 0;
}

AsyncFileLogger:: AsyncFileLogger() {}

AsyncFileLogger:: ~AsyncFileLogger() {
    stop_flag_ = true;
    if (thread_ptr_) {
        stop_flag_ = true;
        cond_.notify_one();
        thread_ptr_->join();
    }
    // 这里日志线程已经结束了
    std::lock_guard<std::mutex> lk(mut_);
    if (!log_buffer_.empty()) {
        WriteLogToFile(log_buffer_);
    }
}

void AsyncFileLogger::Output(const std::stringstream& out) {
    std::lock_guard<std::mutex> lk(mut_);
    log_buffer_.append(out.str());

    // 只有当缓冲区长度到达一定长度时才唤醒日志线程
    if (log_buffer_.length() > MEM_BUFFER_SIZE) {
        cond_.notify_one();
    }
}

void AsyncFileLogger::StartLogging() {
    thread_ptr_ = std::unique_ptr<std::thread>(new std::thread(std::bind(&AsyncFileLogger::LogThreadFunc, this)));
}

void AsyncFileLogger::SetFileName(const std::string& base_name, const std::string& ext_name,
        const std::string& path) {
     file_base_name_ = base_name;
     ext_name[0] == '.' ? file_ext_name_ = ext_name : file_ext_name_ = std::string(".") + ext_name;
     file_path_ = path;
     if (file_path_.empty()) {
        file_path_ = "./";
     }
     if (file_path_.back() != '/') {
        file_path_ = file_path_ + "/";
     }
}

void AsyncFileLogger::WriteLogToFile(const std::string& buf) {
    if (!logger_file_ptr_) {
        logger_file_ptr_ = std::unique_ptr<LoggerFile>(new LoggerFile(file_path_, file_base_name_, file_ext_name_));
    }
    logger_file_ptr_->WriteLog(buf);
    // 如果日志文件过大，就重新 open 一个新的 日志文件
    if (logger_file_ptr_->GetLength() > size_limit_) {
        logger_file_ptr_.reset();
    }
}

void AsyncFileLogger::LogThreadFunc() {
    while (!stop_flag_) {
        std::unique_lock<std::mutex> lk(mut_);
        cond_.wait_for(lk, std::chrono::seconds(LOG_FLUSH_TIMEOUT), [=]() { return !log_buffer_.empty(); });
        // 减小临界区
        std::string tmp_str;
        tmp_str.swap(log_buffer_);
        lk.unlock();
        // 写日志
        WriteLogToFile(tmp_str);
    }
}

}   // namespace water
