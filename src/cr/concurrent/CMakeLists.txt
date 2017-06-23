
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_concurrent_SRCS
    pipe.h
    spawn.h
    spawn_coro.h
    spawn.cpp
)

set(cr_concurrent_HEADERS
    pipe.h
    spawn.h
    spawn_coro.h
)

add_library(cr_concurrent ${cr_concurrent_SRCS})
install(FILES ${cr_concurrent_HEADERS} DESTINATION include/cr/concurrent)
install(TARGETS cr_concurrent DESTINATION lib)