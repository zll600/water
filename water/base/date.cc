#include "date.h"

#include <sys/time.h>

#define MICOR_SECONDS_PER_SEC   1000000

namespace water {

Date Date::After(double seconds) const {
    return Date(micro_seconds_since_epoch_ + seconds * MICOR_SECONDS_PER_SEC);
}

const Date Date::Now() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return Date(seconds * MICOR_SECONDS_PER_SEC + tv.tv_usec);
}

} // namespace water
