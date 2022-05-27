#include "water/water.h"

int main() {
    LOG_DEBUG << "debug log! " << 1; 
    LOG_TRACE << "trace log! " << 2;
    LOG_INFO << "info log! " << 3;
    LOG_WARN << "warning log! " << 4;

    LOG_ERROR << "error log! " << 5;
    LOG_FATAL << "fatal log! " << 6;
    FILE *fp = fopen("./notexistfile", "rb");
    if (fp == nullptr) {
        LOG_SYSERR << "syerr log " << 7;
    }
}
