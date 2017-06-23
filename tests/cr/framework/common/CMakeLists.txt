
find_package(Boost REQUIRED unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_framework_common_unittest_SRCS
    path_directory_unittest.cpp
    unittest_main.cpp
)
  
add_executable(cr_framework_common_unittest ${cr_framework_common_unittest_SRCS})
install(TARGETS cr_framework_common_unittest DESTINATION bin)
target_link_libraries(cr_framework_common_unittest
    cr_framework_common
    cr_common
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)
add_test(cr_framework_common_unittest cr_framework_common_unittest)