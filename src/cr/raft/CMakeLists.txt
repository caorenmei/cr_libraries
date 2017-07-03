
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(PROTO_LIST raft_msg.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_LIST})  

set(cr_raft_SRCS
    ${PROTO_HDRS} ${PROTO_SRCS}
    candidate.h
    candidate.cpp
    exception.h
    follower.h
    follower.cpp
    leader.h
    leader.cpp
    storage.h
    storage.cpp
    mem_storage.h
    mem_storage.cpp
    raft_engine.h
    raft_engine.cpp
    raft_engine_builder.h
    raft_engine_builder.cpp
    raft_state.h
    raft_state.cpp
)

set(cr_raft_HEADERS
    ${PROTO_HDRS}
    exception.h
    log_storage.h
    mem_log_storage.h
    raft_engine.h
    raft_engine_builder.h
)
 
add_library(cr_raft ${cr_raft_SRCS})
install(FILES ${cr_raft_HEADERS} DESTINATION include/cr/raft)
install(TARGETS cr_raft DESTINATION lib)