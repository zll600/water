# water

- Noncopyable
    - 这个类提供给其他类来继承，这里只允许子类进行构造，不能构造这个类的实例 
- Date
    - 这个类以微秒的形式返回保存的时间
- Timer
    - 这个类实现定时器，可以周期执行
- TimerQueue
    - 定时器集合，用来统一管理定时器，使用 timerfd 来将定时器的触发注册到 Poller 中
- Channel
    - 一个 channel 负责一个文件描述符的事件，注册到 Poller 中
- Poller
    - 封装 epoll，实现 IO 多路复用
- EventLoop
    - 封装事件循环
