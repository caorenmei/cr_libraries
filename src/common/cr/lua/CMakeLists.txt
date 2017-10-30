
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

find_package(Selene REQUIRED)
include_directories(${SELENE_INCLUDE_DIRS})

set(cr_common_lua_SRCS
    check_type.h
    log_params.h
    log_params.cpp
    optional.h
    protobuf.h
    protobuf.cpp
    protobuf_pool.h
    protobuf_pool.cpp
    reference.h
    string.h
    string.cpp
    vector.h
)

add_library(cr_common_lua ${cr_common_lua_SRCS})
    