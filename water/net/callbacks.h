#ifndef __WATER_NET_CALLBACKS_H__
#define __WATER_NET_CALLBACKS_H__

#include <functional>
#include <memory>

namespace water {

using TimerCallback = std::function<void()>;

// the data has been read to (buf, len)
class TcpConnection;
class MsgBuffer;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using RecvMessageCallback = std::function<void(const TcpConnectionPtr&, MsgBuffer*)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

}   // namespace water

#endif // __WATER_NET_CALLBACKS_H__
