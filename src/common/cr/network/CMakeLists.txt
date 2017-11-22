
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_common_network_SRCS
    acceptor.h
    acceptor.cpp
    byte_buffer_stream.h
    byte_buffer_stream.cpp
    byte_buffer.h
    byte_buffer.cpp
    connector.h
    connector.cpp
    connection.h
    connection.cpp
    protobuf_codec.h
    protobuf_codec.cpp
)

add_library(cr_common_network ${cr_common_network_SRCS})