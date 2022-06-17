#include "water/water.h"

#include <cstdlib>

#include <iostream>

int main() {
    water::EventLoop loop;
    LOG_FATAL << water::Date::Now().RoundDay().get_micro_seconds_since_epoch();
    water::Date begin = water::Date::Now().RoundSecond().After(2);
    loop.RunAt(begin, [=,&loop](){
        LOG_DEBUG<<"test begin:";

        srand(unsigned(time(NULL)));
        for(int i = 0;i < 10000; i++)
        {
            int aa = rand() % 10000;
            double s = (double)aa / 1000.0+1;
            loop.RunAt(begin.After(s), [=](){
                LOG_ERROR <<"run After:" << s;
            });
        }
        LOG_DEBUG <<"timer created!";
    });

    loop.Loop();
}
