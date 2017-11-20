
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(CppNetLib REQUIRED)
include_directories(${CPPNETLIB_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

set(cr_common_zk_PROTOS service.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${cr_common_zk_PROTOS})  
source_group("Proto Files" FILES ${cr_common_zk_PROTOS})

set(cr_common_zk_SRCS
    ${PROTO_HDRS} ${PROTO_SRCS}
    create_model.h
    create_model.cpp
    data_node.h
    data_node.cpp
    data_node_stat.h
    data_node_stat.cpp
    data_tree.h
    data_tree.cpp
    exceptions.h
    session.h
    session.cpp
    watcher.h
    watcher.cpp
    watch_manager.h
    watch_manager.cpp
)

add_library(cr_common_zk ${cr_common_zk_SRCS} ${cr_common_zk_PROTOS})
target_link_libraries(cr_common_zk cr_common_raft)