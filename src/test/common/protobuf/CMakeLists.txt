include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(PROTO_LIST unittest.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_LIST})  

set(cr_common_protobuf_unittest_SRCS
    ${PROTO_HDRS}
    ${PROTO_SRCS}
    any_unittest.cpp
    reflect_unittest.cpp
    unittest_main.cpp
)

source_group("Protobuf Files" FILES ${PROTO_LIST})
  
add_executable(cr_common_protobuf_unittest ${cr_common_protobuf_unittest_SRCS} ${PROTO_LIST})
install(TARGETS cr_common_protobuf_unittest DESTINATION bin)
target_link_libraries(cr_common_protobuf_unittest
    cr_common_core
    cr_common_protobuf
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${PROTOBUF_LIBRARIES}
)
add_test(cr_common_protobuf_unittest cr_common_protobuf_unittest)