
find_package(Boost REQUIRED system unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_raft_unittest_SRCS
    unittest_main.cpp
    mem_log_storage_unittest.cpp
)
  
add_executable(cr_raft_unittest ${cr_raft_unittest_SRCS})
install(TARGETS cr_raft_unittest DESTINATION bin)
target_link_libraries(cr_raft_unittest 
    cr_raft
    cr_common
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
add_test(cr_raft_unittest cr_raft_unittest)