find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_common_app_SRCS
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

set(cr_common_app_HEADERS
    application.h
    exceptions.h
    cluster.h
    message.h
    service.h
)

add_library(cr_common_app ${cr_common_app_SRCS})