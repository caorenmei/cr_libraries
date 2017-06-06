
find_package(Boost REQUIRED unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_common_unittest_SRCS
    unittest_main.cpp
    assert_unittest.cpp
    error_unittest.cpp
    exception_unittest.cpp
    scope_guard_unittest.cpp
    throw_unittest.cpp
)
  
add_executable(cr_common_unittest ${cr_common_unittest_SRCS})
install(TARGETS cr_common_unittest DESTINATION bin)
target_link_libraries(cr_common_unittest 
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    cr_common
)
add_test(cr_common_unittest cr_common_unittest)