include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(CppNetLib REQUIRED)
include_directories(${CPPNETLIB_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

set(cr_common_app_PROTOS name_msg.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${cr_common_app_PROTOS})  
source_group("Proto Files" FILES ${cr_common_app_PROTOS})

set(cr_common_app_SRCS
    ${PROTO_SRCS} ${PROTO_HDRS}
    application.h
    application.cpp
    cluster.h
    cluster_proxy_impl.h
    cluster_proxy_impl.cpp
    exceptions.h
    local_cluster_impl.h
    local_cluster_impl.cpp
    message.h
    name_service.h
    name_service.cpp
    raft_service.h
    raft_service.cpp
    service.h
    service.cpp
)

add_library(cr_common_app ${cr_common_app_SRCS} ${cr_common_app_PROTOS})
add_dependencies(cr_common_app cr_common_raft)