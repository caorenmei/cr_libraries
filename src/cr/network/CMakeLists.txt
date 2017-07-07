
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_network_SRCS
    byte_buffer.h
    byte_buffer.cpp
    protobuf_utils.h
    protobuf_utils.cpp
)

set(cr_network_HEADERS
    byte_buffer.h
    protobuf_utils.h
)

add_library(cr_network ${cr_network_SRCS})
install(FILES ${cr_network_HEADERS} DESTINATION include/cr/network)
install(TARGETS cr_network DESTINATION lib)