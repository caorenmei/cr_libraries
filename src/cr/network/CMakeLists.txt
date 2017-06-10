
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_network_SRCS
    pipe.h
    spawn.h
    spawn_coro.h
    spawn.cpp
)

set(cr_network_HEADERS
    pipe.h
    spawn.h
    spawn_coro.h
)

add_library(cr_network ${cr_network_SRCS})
install(FILES ${cr_network_HEADERS} DESTINATION include/cr/network)
install(TARGETS cr_network DESTINATION lib)