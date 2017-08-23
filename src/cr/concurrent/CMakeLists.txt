
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_concurrent_SRCS
    multi_mutex.h
    pipe.h
    spawn.h
    spawn_coro.h
    spawn.cpp
    thread.h
    thread.cpp
)

set(cr_concurrent_HEADERS
    multi_mutex.h
    pipe.h
    spawn.h
    spawn_coro.h
    thread.h
)

add_library(cr_concurrent ${cr_concurrent_SRCS})
install(FILES ${cr_concurrent_HEADERS} DESTINATION include/cr/concurrent)
install(TARGETS cr_concurrent DESTINATION lib)