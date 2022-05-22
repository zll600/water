#include "water/water.h"
#include <iostream>

int main(void) {
    const water::Date obj = water::Date::Now();
    std::cout << obj.get_micro_seconds_since_epoch() << std::endl;
    water::Date new_obj = obj.After(100);
    std::cout << new_obj.get_micro_seconds_since_epoch() << std::endl;

    return 0;
}
