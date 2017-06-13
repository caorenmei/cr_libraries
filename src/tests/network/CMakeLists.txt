
find_package(Boost REQUIRED chrono context coroutine date_time regex system thread unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_network_unittest_SRCS
    byte_buffer_unittest.cpp
    pipe_unittest.cpp
    spawn_unittest.cpp
    unittest_main.cpp
)
  
add_executable(cr_network_unittest ${cr_network_unittest_SRCS})
install(TARGETS cr_network_unittest DESTINATION bin)
target_link_libraries(cr_network_unittest
    cr_common
    cr_network
    ${Boost_COROUTINE_LIBRARY} 
    ${Boost_CONTEXT_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY} 
    ${Boost_REGEX_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_THREAD_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${_CR_PTHREAD_LIBRARY}
)
add_test(cr_network_unittest cr_network_unittest)