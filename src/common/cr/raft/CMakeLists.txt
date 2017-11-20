
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(CppNetLib REQUIRED)
include_directories(${CPPNETLIB_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

set(cr_common_raft_PROTOS raft_msg.proto service.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${cr_common_raft_PROTOS})  
source_group("Proto Files" FILES ${cr_common_raft_PROTOS})

set(cr_common_raft_SRCS
    ${PROTO_HDRS} ${PROTO_SRCS}
    buddy_node.h
    buddy_node.cpp
    exception.h
    file_storage.h
    file_storage.cpp
    mem_storage.h
    mem_storage.cpp
    options.h
    options.cpp
    storage.h
    raft.h
    raft.cpp
    raft_node.h
    raft_node.cpp
    service.h
    service.cpp
    raft_state.h
    raft_state.cpp
    service_options.h
    service_options.cpp
)

add_library(cr_common_raft ${cr_common_raft_SRCS} ${cr_common_raft_PROTOS})