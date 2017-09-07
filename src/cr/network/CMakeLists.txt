
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_network_SRCS
    byte_buffer.h
    byte_buffer.cpp
    pb_connection.h
    pb_connection.cpp
    pb_serialize.h
    pb_serialize.cpp
)

set(cr_network_HEADERS
    byte_buffer.h
    pb_connection.h
    pb_serialize.h
)

add_library(cr_network ${cr_network_SRCS})
install(FILES ${cr_network_HEADERS} DESTINATION include/cr/network)
install(TARGETS cr_network DESTINATION lib)