include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

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
    service.h
    service.cpp
)

add_library(cr_common_app ${cr_common_app_SRCS} ${cr_common_app_PROTOS})