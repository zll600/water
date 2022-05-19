#ifndef __WATER_BASE_DATE_H__
#define __WATER_BASE_DATE_H__

#include <stdint.h>

namespace water {

// 以微秒形式返回的时间
class Date {
 public:
    Date() : micro_seconds_since_epoch_(0) {}
    ~Date() =default;

    // 获取 seconds 之后的时间
    Date After(double seconds) const;
    
    bool operator<(const Date& date) const {
        return micro_seconds_since_epoch_ < date.micro_seconds_since_epoch_;
    }

    bool operator>(const Date& date) const {
        return micro_seconds_since_epoch_ > date.micro_seconds_since_epoch_;
    }

    int64_t get_micro_seconds_since_epoch() {
        return micro_seconds_since_epoch_;
    }

    // 获取当前时间
    static const Date Now();

 private:
    Date(int64_t micro_seconds) : micro_seconds_since_epoch_(micro_seconds) {}

    int64_t micro_seconds_since_epoch_ = 0;     // 以微秒形式保存时间
};

} // namespace water

#endif // __WATER_BASE_DATE_H__
