
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(PROTO_LIST raft_msg.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_LIST})  

set(cr_raft_SRCS
    ${PROTO_HDRS} ${ROTO_SRCS}
    log_entry.h
    log_entry.cpp
    log_storage.h
    log_storage.cpp
    mem_log_storage.h
    mem_log_storage.cpp
    node.h
    node.cpp
    raft_engine.h
    raft_engine.cpp
)

set(cr_raft_HEADERS
    ${PROTO_HDRS}
    log_entry.h
    log_storage.h
    mem_log_storage.h
    node.h
    raft_engine.h
)
 
add_library(cr_raft ${cr_raft_SRCS})
install(FILES ${cr_raft_HEADERS} DESTINATION include/cr/raft)
install(TARGETS cr_raft DESTINATION lib)