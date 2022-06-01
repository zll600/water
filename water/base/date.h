#ifndef __WATER_BASE_DATE_H__
#define __WATER_BASE_DATE_H__

#include <stdint.h>

#include <string>

#define MICRO_SECONDS_PER_SEC 1000000

namespace water {

// 以微秒形式返回的时间
class Date {
 public:
    Date() : micro_seconds_since_epoch_(0) {}
    ~Date(){}

    // 获取 seconds 之后的时间
    const Date After(double seconds) const;
    
    bool operator<(const Date& date) const {
        return micro_seconds_since_epoch_ < date.micro_seconds_since_epoch_;
    }

    bool operator>(const Date& date) const {
        return micro_seconds_since_epoch_ > date.micro_seconds_since_epoch_;
    }

    int64_t get_micro_seconds_since_epoch() const {
        return micro_seconds_since_epoch_;
    }

    // 格式化时间
    std::string ToFormattedString(bool show_micro_seconds) const; // UTC
    std::string ToCustomedFormattedString(const std::string& fmt_str) const; // UTC
     
    // 判断时间是否在同一秒
    bool IsSameSecond(const Date& date) const {
        return micro_seconds_since_epoch_ / MICRO_SECONDS_PER_SEC == date.get_micro_seconds_since_epoch() / MICRO_SECONDS_PER_SEC;
    }

    // 对秒取整
    const Date RoundSecond() const;
    // 对天取整
    const Date RoundDay() const;

    // 获取当前时间
    static const Date Now();

 private:
    Date(int64_t micro_seconds) : micro_seconds_since_epoch_(micro_seconds) {}

    int64_t micro_seconds_since_epoch_ = 0;     // 以微秒形式保存时间
};

} // namespace water

#endif // __WATER_BASE_DATE_H__
