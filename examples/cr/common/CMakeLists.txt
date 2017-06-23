
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# assert.h
add_executable(cr_common_example_assert assert_example.cpp)
install(TARGETS cr_common_example_assert DESTINATION bin)
target_link_libraries(cr_common_example_assert cr_common)