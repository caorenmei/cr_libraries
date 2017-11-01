
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

set(cr_common_raft_PROTOS raft_msg.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${cr_common_raft_PROTOS})  
source_group("Proto Files" FILES ${cr_common_raft_PROTOS})

set(cr_common_raft_SRCS
    ${PROTO_HDRS} ${PROTO_SRCS}
    exception.h
    file_storage.h
    file_storage.cpp
    storage.h
    mem_storage.h
    mem_storage.cpp
    raft.h
    raft.cpp
    raft_state.h
    raft_state.cpp
)

add_library(cr_common_raft ${cr_common_raft_SRCS} ${cr_common_raft_PROTOS})