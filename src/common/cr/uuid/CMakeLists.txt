
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

if(NOT MSVC)
    find_package(Uuid REQUIRED)
    include_directories(${UUID_INCLUDE_DIRS})
endif()

set(cr_common_uuid_SRCS
    uuid.h
    win_uuid.cpp
    unix_uuid.cpp
)

add_library(cr_common_uuid ${cr_common_uuid_SRCS})
    