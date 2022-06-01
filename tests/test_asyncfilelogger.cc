#include "water/water.h"

#include <stdio.h>
#include <thread>

int main() {
    water::AsyncFileLogger async_file_logger;
    async_file_logger.SetFileName("async_test");

    water::Logger::set_output_func([&](const std::stringstream& buf) {
        async_file_logger.Output(buf);
    }, [](){});
    LOG_DEBUG << "debug log!" << 1;
    LOG_DEBUG << "trace log!" << 2;
    LOG_INFO << "info log!" << 3;
    LOG_WARN << "warn log!" << 4;
    LOG_ERROR << "error log!" << 5;
    std::thread thr([]() {
        LOG_FATAL << "fatal log!" << 6;
    });

    FILE *fp = fopen("/notexistfile", "rb");
    if (fp == nullptr) {
        LOG_SYSERR << "syserr log1" << 7;
    }
    thr.join();

    return 0;
}
