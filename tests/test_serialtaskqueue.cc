#include "water/water.h"

#include <cstdio>
#include <unistd.h>

int main() {
    water::SerialTaskQueue que1("test_queue");
    water::SerialTaskQueue que2("");

    que1.RunTaskInQueue([&](){
        for (int i = 0; i < 5; ++i) {
            sleep(1);
            printf("task(%s) i = %d\n", que1.get_queue_name().c_str(), i);
        }
    });

    que2.RunTaskInQueue([&]() {
        for (int i = 0; i < 5; ++i) {
            sleep(1);
            printf("task(%s) i = %d\n", que2.get_queue_name().c_str(), i);
        }
    });

    que1.WaitAllTaskFinished();
    que2.WaitAllTaskFinished();
    return 0;
}
