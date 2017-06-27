
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
    error_code.h
    exception.h
    exception.cpp
    follower.h
    follower.cpp
    leader.h
    leader.cpp
    log_entry.h
    log_entry.cpp
    log_storage.h
    mem_log_storage.h
    mem_log_storage.cpp
    raft_engine.h
    raft_engine.cpp
    raft_engine_builder.h
    raft_engine_builder.cpp
    raft_state.h
    raft_state.cpp
    replay.h
    replay.cpp
    state_machine.h
)

set(cr_raft_HEADERS
    ${PROTO_HDRS}
    error_code.h
    exception.h
    log_entry.h
    log_storage.h
    mem_log_storage.h
    raft_engine.h
    raft_engine_builder.h
    state_machine.h
)
 
add_library(cr_raft ${cr_raft_SRCS})
install(FILES ${cr_raft_HEADERS} DESTINATION include/cr/raft)
install(TARGETS cr_raft DESTINATION lib)