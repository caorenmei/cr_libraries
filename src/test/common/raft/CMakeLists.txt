
find_package(Boost REQUIRED atomic date_time filesystem log system thread unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Rocksdb REQUIRED)
include_directories(${ROCKSDB_INCLUDE_DIRS})

set(cr_common_raft_unittest_SRCS
    unittest_main.cpp
    file_storage_unittest.cpp
    mem_storage_unittest.cpp
)
  
add_executable(cr_common_raft_unittest ${cr_common_raft_unittest_SRCS})
install(TARGETS cr_common_raft_unittest DESTINATION bin)
if (WIN32)
    find_library(WIN_RPCRT_LIBRARY Rpcrt4)
endif()
target_link_libraries(cr_common_raft_unittest 
    cr_common_raft
    cr_common_core
    ${Boost_LOG_LIBRARY}
    ${Boost_ATOMIC_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${ROCKSDB_LIBRARIES}
    ${WIN_RPCRT_LIBRARY}
)
add_test(cr_common_raft_unittest cr_common_raft_unittest)