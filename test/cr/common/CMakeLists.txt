
find_package(Boost REQUIRED atomic chrono date_time filesystem log log_setup thread system unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_common_unittest_SRCS
    unittest_main.cpp
    assert_unittest.cpp
    error_unittest.cpp
    exception_unittest.cpp
    logging_unittest.cpp
    scope_guard_unittest.cpp
    streams_unittest.cpp
    throw_unittest.cpp
)
  
add_executable(cr_common_unittest ${cr_common_unittest_SRCS})
install(TARGETS cr_common_unittest DESTINATION bin)
target_link_libraries(cr_common_unittest 
    cr_common
    ${Boost_LOG_LIBRARY} 
    ${Boost_LOG_SETUP_LIBRARY} 
    ${Boost_ATOMIC_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_THREAD_LIBRARY} 
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)
add_test(cr_common_unittest cr_common_unittest)