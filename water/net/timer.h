#ifndef __WATER_BASE_TIMER_H__
#define __WATER_BASE_TIMER_H__

#include <atomic>
#include <stdint.h> 

#include "water/base/noncopyable.h"
#include "water/base/date.h"
#include "callbacks.h"

namespace water {

// 定时器
class Timer : Noncopyable {
 public:
    Timer(const TimerCallback& cb, const Date& when, double interval)
        : callback_(cb),
        when_(when),
        interval_(interval),
        repeat_(interval - 0.0 > 0.0000001),
        timer_seq_(timer_created_++) {
    }

    ~Timer() {}

    // 执行回调函数
    void Run() const;
    // 重置定时器时间
    // 只有周期执行的定时器才可以重新设置
    // 非周期执行的定时器设置为无效值
    void Restart(const Date& now);
    bool operator<(const Timer& timer) const;
    bool operator>(const Timer& timer) const;
    const Date& get_when() const { return when_; }
    bool IsRepeat() { return repeat_; }

 private:
    TimerCallback callback_;    // 定时器回调函数
    Date when_;                 // 到期时间
    const double interval_;     // s执行周期
    const bool repeat_;         // 是否重复
    const int64_t timer_seq_;    // 定时器序列号
    static std::atomic<int64_t> timer_created_;  // 序列号生成器
};

} // namespace water

#endif // __WATER_BASE_TIMER_H__
