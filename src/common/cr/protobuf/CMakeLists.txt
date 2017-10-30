
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_common_protobuf_SRCS
    any.h
    any.cpp
    reflect.h
    reflect.cpp
)

set(cr_common_protobuf_HEADERS
    any.h
    reflect.h
)

add_library(cr_common_protobuf ${cr_common_protobuf_SRCS})