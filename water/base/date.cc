#include "date.h"

#include <time.h>
#include <sys/time.h>

namespace water {

const Date Date::After(double seconds) const {
    return Date(micro_seconds_since_epoch_ + seconds * MICRO_SECONDS_PER_SEC);
}

const Date Date::Now() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return Date(seconds * MICRO_SECONDS_PER_SEC + tv.tv_usec);
}

std::string Date::ToFormattedString(bool show_micro_seconds) const {
    char buf[128] = {0};
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / MICRO_SECONDS_PER_SEC);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (show_micro_seconds) {
        int micro_seconds = static_cast<int>(micro_seconds_since_epoch_ % MICRO_SECONDS_PER_SEC);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, micro_seconds);
    } else {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

const Date Date::RoundSecond() const {
    return Date(micro_seconds_since_epoch_ - (micro_seconds_since_epoch_ % MICRO_SECONDS_PER_SEC));
}

const Date Date::RoundDay() const {
    struct tm *t;
    time_t seconds = static_cast<time_t>(micro_seconds_since_epoch_ / MICRO_SECONDS_PER_SEC);
    t = localtime(&seconds);
    t->tm_hour = 0;
    t->tm_min = 0;
    t->tm_sec = 0;
    return Date(mktime(t) * MICRO_SECONDS_PER_SEC);
}

} // namespace water
