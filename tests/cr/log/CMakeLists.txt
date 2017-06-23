
find_package(Boost REQUIRED atomic chrono date_time filesystem log log_setup thread system unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_log_unittest_SRCS
    unittest_main.cpp
    logging_unittest.cpp
)
  
add_executable(cr_log_unittest ${cr_log_unittest_SRCS})
install(TARGETS cr_log_unittest DESTINATION bin)
target_link_libraries(cr_log_unittest 
    cr_log
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
add_test(cr_log_unittest cr_log_unittest)