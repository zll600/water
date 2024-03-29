#include "water/water.h"

#include <stdio.h>
#include <thread>

int main() {
    water::AsyncFileLogger async_file_logger;
    async_file_logger.SetFileName("async_test");
    async_file_logger.StartLogging();

    water::Logger::set_output_func([&](const char *msg, const uint64_t len) {
        async_file_logger.Output(msg, len);
    }, [&](){ async_file_logger.Flush(); });

    async_file_logger.set_file_size_limit(100000000);

    /*
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
    */
    
//     std::thread thr2([]() {
        for (int i = 0; i < 1000000; ++i) {
            if (i % 100 == 0) {
                LOG_ERROR << "this is the " << i << "th log";
                continue;
            }
            LOG_INFO << "this is the " << i << "th log";
            ++i;
            LOG_DEBUG << "this is the " << i << "th log";
        }
//     });

    // thr.join();
//     thr2.join();


    return 0;
}
