#include "water/water.h"
#include <iostream>

int main(void) {
    LOG_DEBUG << "test start";
    water::Logger::set_log_level(water::Logger::TRACE);
    // water::EventLoop loop;
    water::EventLoopThread loop_thread;
    loop_thread.Run();
    water::InetAddress addr(8888);
    water::TcpServer server(loop_thread.get_loop(), addr, "test");
    server.set_recv_msg_callback([](const water::TcpConnectionPtr& conn, water::MsgBuffer *buffer){
        LOG_DEBUG<<"recv callback!";
        std::cout << std::string(buffer->Peek(), buffer->ReadableBytes());
        conn->Send(buffer->Peek(),buffer->ReadableBytes());
        buffer->RetrieveAll();
        conn->Shutdown();
    });
    server.set_conn_callback([](const water::TcpConnectionPtr& conn) {
        if (conn->IsConnected()) {
            LOG_DEBUG << "New Connection";
        } else if (conn->IsDisconnected()) {
            LOG_DEBUG << "connection desconnected";
        }
    });
    server.Start();
    // loop.Loop();
    loop_thread.Wait();

    return 0;
}
