
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_common_concurrent_SRCS
    multi_mutex.h
    pipe.h
    spawn.h
    spawn_coro.h
    spawn.cpp
    thread.h
    thread.cpp
)

set(cr_common_concurrent_HEADERS
    multi_mutex.h
    pipe.h
    spawn.h
    spawn_coro.h
    thread.h
)

add_library(cr_common_concurrent ${cr_common_concurrent_SRCS})