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
- TaskQueue
    - 任务队列，只定义接口
- SerialTaskQueue
    - 串行的任务队列
- Logger
    - 日志类，支持不同的日志输出函数，可以选择多种输出地和刷新缓冲区的方式
- AsyncLogger
    - LoggerFile
        - 封装对 FILE 的操作
    - 实现异步文件日志
