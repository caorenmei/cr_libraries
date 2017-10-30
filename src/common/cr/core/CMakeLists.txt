
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_common_core_SRCS
    assert.h
    assert_builder.h
    assert_error.h
    consistent_hash.h
    scope_guard.h
    scope_guard.cpp
    error.h
    exception.h
    exception.cpp
    function.h
    heap.h
    logging.h
    optional.h
    streams.h
    streams_empty.h
    streams_enumerators.h
    streams_from.h
    streams_for.h
    streams_generate.h
    throw.h
    tuple_utils.h
    type_utils.h
)

set(cr_common_core_HEADERS
    assert.h
    assert_builder.h
    assert_error.h
    consistent_hash.h
    scope_guard.h
    error.h
    exception.h
    function.h
    heap.h
    logging.h
    streams.h
    streams_empty.h
    streams_enumerators.h
    streams_from.h
    streams_for.h
    streams_generate.h
    throw.h
    tuple_utils.h
    type_utils.h
)
 
add_library(cr_common_core ${cr_common_core_SRCS})