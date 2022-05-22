include_directories(
    ${PROJECT_SOURCE_DIR}
)

set(LIB_SRC
    base/date.cc
    net/timer.cc
)

add_library(water ${LIB_SRC})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
