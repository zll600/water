#include "async_file_logger.h"

#include <cstdio>
#include <cstring>
#include <sys/prctl.h>

#include <iostream>
#include <sstream>
#include <functional>
#include <chrono>

#define LOG_FLUSH_TIMEOUT 1
#define MEM_BUFFER_SIZE 4 * 1024 * 1024

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
        char seq[10];
        sprintf(seq, ".%06ld", file_seq_ % 1000000);
        file_seq_++;
        std::string new_name = file_path_ + file_base_name_ + "." + create_date_.ToCustomedFormattedString("%y%m%d-%H%M%S") + std::string(seq) + file_ext_name_;
        rename(file_full_name_.c_str(), new_name.c_str());
    }   
}

uint64_t AsyncFileLogger::LoggerFile::file_seq_ = 0;
        
void AsyncFileLogger::LoggerFile::WriteLog(const StringPtr buf) {
    if (fp_) {
        fwrite(buf->c_str(),1,buf->length(),fp_);
    }
}

uint64_t AsyncFileLogger::LoggerFile::GetLength() const {
    if (fp_) {
        return ftell(fp_);
    }
    return 0;
}

AsyncFileLogger::AsyncFileLogger() 
    : log_buffer_ptr_(new std::string),
    next_buffer_ptr_(new std::string) {
    log_buffer_ptr_->reserve(MEM_BUFFER_SIZE);
    next_buffer_ptr_->reserve(MEM_BUFFER_SIZE);
}

AsyncFileLogger:: ~AsyncFileLogger() {
    std::cout << "AsyncFileLogger::~AsyncFileLogger()" << std::endl;
    stop_flag_ = true;
    if (thread_ptr_) {
        cond_.notify_all();
        thread_ptr_->join();
        std::cout << "async_file_logger thread exit" << std::endl;
    }
    // 这里日志线程已经结束了
    std::lock_guard<std::mutex> lk(mut_);
    if (!log_buffer_ptr_->empty()) {
        write_buffers_.push(log_buffer_ptr_);
    }
    if (!write_buffers_.empty()) {
        StringPtr tmp_ptr = std::move(write_buffers_.front());
        write_buffers_.pop();

        WriteLogToFile(tmp_ptr);
    }
}

void AsyncFileLogger::Output(const char *msg, uint64_t len) {
    std::lock_guard<std::mutex> lk(mut_);

    if (len > MEM_BUFFER_SIZE) {
        return;
    }
    
    if (!log_buffer_ptr_) {
        log_buffer_ptr_ = std::make_shared<std::string>();
        log_buffer_ptr_->reserve(MEM_BUFFER_SIZE);
    }

    // 只有当缓冲区长度到达一定长度时才唤醒日志线程
    if (log_buffer_ptr_->capacity() - log_buffer_ptr_->length() < len) {
        write_buffers_.push(log_buffer_ptr_);
        if (next_buffer_ptr_) {
            log_buffer_ptr_ = next_buffer_ptr_;
            next_buffer_ptr_.reset();
        } else {
            log_buffer_ptr_ = std::make_shared<std::string>();
            log_buffer_ptr_->reserve(MEM_BUFFER_SIZE);
        }
        cond_.notify_one();
    }
    
    // 100M bytes log in buffer
    if (write_buffers_.size() > 25) {
        lost_counter_++;
        return;
    }

    if (lost_counter_ > 0) {
        char log_err[128];
        sprintf(log_err, "%ld log information is lost\n", lost_counter_);
        lost_counter_ = 0;
        log_buffer_ptr_->append(log_err);
    }

    log_buffer_ptr_->append(std::string(msg, len));
}

void AsyncFileLogger::Flush() {
    std::lock_guard<std::mutex> lk(mut_);
    if (!log_buffer_ptr_->empty()) {
        SwapBuffer();
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

void AsyncFileLogger::WriteLogToFile(const StringPtr buf) {
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
    prctl(PR_SET_NAME, "AsyncFileLogger");
    while (!stop_flag_) {
        {
            std::unique_lock<std::mutex> lk(mut_);
            while (write_buffers_.empty() && !stop_flag_) {
                if (cond_.wait_for(lk, std::chrono::seconds(LOG_FLUSH_TIMEOUT)) == std::cv_status::timeout) {
                    if (!log_buffer_ptr_->empty()) {
                        SwapBuffer();
                    }
                    break;
                }
            }
            tmp_buffers_.swap(write_buffers_);
        }
        while (!write_buffers_.empty()) {
            StringPtr tmp_ptr = std::move(write_buffers_.front());
            write_buffers_.pop();
            WriteLogToFile(tmp_ptr);
            tmp_ptr->clear();
            {
                std::unique_lock<std::mutex> lk(mut_);
                next_buffer_ptr_ = tmp_ptr;
            }
        }
    }
}

void AsyncFileLogger::SwapBuffer() {
    write_buffers_.push(log_buffer_ptr_);
    if (next_buffer_ptr_) {
        log_buffer_ptr_ = next_buffer_ptr_;
        next_buffer_ptr_.reset();
        log_buffer_ptr_->clear();
    } else {
        log_buffer_ptr_ = std::make_shared<std::string>();
        log_buffer_ptr_->reserve(MEM_BUFFER_SIZE);
    }
}

}   // namespace water
