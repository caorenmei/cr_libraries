
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_common_SRCS
    assert.h
    assert_builder.h
    assert_builder.cpp
    assert_error.h
    error.h
    error.cpp
    exception.h
    exception.cpp
    scope_guard.h
    throw.h
)

set(cr_common_HEADERS
    assert.h
    assert_builder.h
    assert_error.h
    error.h
    exception.h
    scope_guard.h
    throw.h
)
 
add_library(cr_common ${cr_common_SRCS})
install(FILES ${cr_common_HEADERS} DESTINATION include/cr/common)
install(TARGETS cr_common DESTINATION lib)