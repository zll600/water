#include "water/water.h"
#include <iostream>

int main()
{
    water::EventLoop loop;
    loop.RunEvery(3,[](){
        std::cout<<"runEvery 3s"<<std::endl;
    });
    loop.RunAt(water::Date::Now().After(10),[](){
        std::cout<<"runAt 10s later"<<std::endl;
    });
    loop.RunAfter(5,[](){
        std::cout<<"runAt 5s later"<<std::endl;
    });
    /*
    loop.RunEvery(1,[](){
        std::cout<<"runEvery 1s"<<std::endl;
    });
    */
    loop.RunAfter(4,[](){
        std::cout<<"runAfter 4s"<<std::endl;
    });
    loop.Loop();

    return 0;
} 
