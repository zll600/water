#include "timer.h"

namespace water {
    std::atomic<int64_t> Timer::timer_created_;

    void Timer::Run() const {
        callback_();
    }

    void Timer::Restart(const Date& now) {
        if (repeat_) {
            when_ = now.After(interval_);
        } else {
            when_ = Date();
        }
    }

    bool Timer::operator<(const Timer& timer) const {
        return when_ < timer.when_;
    }

    bool Timer::operator>(const Timer& timer) const {
        return when_ > timer.when_;
    }
}   // namespace water
