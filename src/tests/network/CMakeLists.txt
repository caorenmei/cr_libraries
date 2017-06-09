
find_package(Boost REQUIRED context coroutine system unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_network_unittest_SRCS
    spawn_unittest.cpp
    unittest_main.cpp
)
  
add_executable(cr_network_unittest ${cr_network_unittest_SRCS})
install(TARGETS cr_network_unittest DESTINATION bin)
target_link_libraries(cr_network_unittest
    ${Boost_CONTEXT}
    ${Boost_COROUTINE}
    ${Boost_SYSTEM}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    cr_common
    cr_network
)
add_test(cr_network_unittest cr_network_unittest)