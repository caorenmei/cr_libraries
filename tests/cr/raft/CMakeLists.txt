
find_package(Boost REQUIRED filesystem system unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

set(cr_raft_unittest_SRCS
    unittest_main.cpp
    candidate_unittest.cpp
    file_storage_unittest.cpp
    follower_unittest.cpp
    mem_storage_unittest.cpp
    leader_unittest.cpp
)
  
add_executable(cr_raft_unittest ${cr_raft_unittest_SRCS})
install(TARGETS cr_raft_unittest DESTINATION bin)
find_library(WIN_RPCRT_LIBRARY Rpcrt4)
target_link_libraries(cr_raft_unittest 
    cr_raft
    cr_common
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${ROCKSDB_LIBRARIES}
    ${WIN_RPCRT_LIBRARY}
)
add_test(cr_raft_unittest cr_raft_unittest)