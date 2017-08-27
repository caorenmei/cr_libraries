
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_protobuf_SRCS
    reflect.h
    reflect.cpp
)

set(cr_protobuf_HEADERS
    reflect.h
)

add_library(cr_protobuf ${cr_protobuf_SRCS})
install(FILES ${cr_protobuf_HEADERS} DESTINATION include/cr/protobuf)
install(TARGETS cr_protobuf DESTINATION lib)