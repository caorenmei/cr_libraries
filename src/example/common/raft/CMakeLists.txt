find_package(Boost REQUIRED unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

set(cr_common_raft_safety_test_SRCS
    server.h
    server.cpp
    safety_test.cpp
)
  
add_executable(cr_common_raft_safety_test ${cr_common_raft_safety_test_SRCS})
install(TARGETS cr_common_raft_safety_test DESTINATION bin)
if (WIN32)
    find_library(WIN_RPCRT_LIBRARY Rpcrt4)
endif()
target_link_libraries(cr_common_raft_safety_test 
    cr_common_raft
    cr_common_core
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${ROCKSDB_LIBRARIES}
    ${WIN_RPCRT_LIBRARY}
)