#include "water/water.h"

int main(void) {
    water::Logger::set_log_level(water::Logger::TRACE);
    water::EventLoop loop;
    water::InetAddress addr("127.0.0.1", 8888);
    water::TcpServer server(&loop, addr, "test");
    server.Start();
    loop.Loop();

    return 0;
}
