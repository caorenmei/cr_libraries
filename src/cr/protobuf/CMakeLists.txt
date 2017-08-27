
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_protobuf_SRCS
    any.h
    any.cpp
    reflect.h
    reflect.cpp
)

set(cr_protobuf_HEADERS
    any.h
    reflect.h
)

add_library(cr_protobuf ${cr_protobuf_SRCS})
install(FILES ${cr_protobuf_HEADERS} DESTINATION include/cr/protobuf)
install(TARGETS cr_protobuf DESTINATION lib)