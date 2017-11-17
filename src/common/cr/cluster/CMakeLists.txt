
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(CppNetLib REQUIRED)
include_directories(${CPPNETLIB_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

set(cr_common_cluster_PROTOS name_service.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${cr_common_cluster_PROTOS})  
source_group("Proto Files" FILES ${cr_common_cluster_PROTOS})

set(cr_common_cluster_SRCS
    ${PROTO_HDRS} ${PROTO_SRCS}
    name_service.h
	name_service.cpp
)

add_library(cr_common_cluster ${cr_common_cluster_SRCS} ${cr_common_cluster_PROTOS})
target_link_libraries(cr_common_cluster cr_common_raft)