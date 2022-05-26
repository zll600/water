#include "water/water.h"

#include <thread>
#include <iostream>

int main(void) {
    water::EventLoop loop;
    std::thread thr([&loop] () {
        std::cout << "RunInLoop called in other thread" << std::endl;
        loop.QueueInLoop([]() {
            std::cout << "QueueInLoop in RunInLoop" << std::endl;
        });
    });

    loop.RunInLoop([]() {
        std::cout << "RunInLoop 1" << std::endl;
    });

    loop.RunInLoop([]() {
        std::cout << "RunInLoop 2" << std::endl;
    });

    loop.RunInLoop([]() {
        std::cout << "RunInLoop 3" << std::endl;
    });

    loop.Loop();

    return 0;
}
