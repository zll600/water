include_directories(
    ${PROJECT_SOURCE_DIR}
)

set(LIB_SRC
    base/date.cc
    net/timer.cc
    net/timer_queue.cc
    net/channel.cc
    net/poller.cc
    net/event_loop.cc
    base/serial_task_queue.cc
    base/logger.cc
    base/async_file_logger.cc
    base/log_stream.cc
)

add_library(water ${LIB_SRC})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
