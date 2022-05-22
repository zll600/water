#include "water/water.h"

#include <iostream>

int main(void) {
    water::Timer timer([] {std::cout << "timer 2s" << std::endl; }, water::Date::Now(), 2.0);
    std::cout << timer.get_when().get_micro_seconds_since_epoch() << std::endl;
    timer.Run();

    timer.Restart(water::Date::Now());
    return 0;
}
