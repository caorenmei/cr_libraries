
find_package(Boost REQUIRED chrono context coroutine date_time regex system thread unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

set(PROTO_LIST unittest.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_LIST})  

set(cr_network_unittest_SRCS
    ${PROTO_HDRS}
    ${PROTO_SRCS}
    byte_buffer_unittest.cpp
    pipe_unittest.cpp
    protobuf_utils_unittest.cpp
    spawn_unittest.cpp
    unittest_main.cpp
)

source_group("Protobuf Files" FILES ${PROTO_LIST})
  
add_executable(cr_network_unittest ${cr_network_unittest_SRCS})
install(TARGETS cr_network_unittest DESTINATION bin)
target_link_libraries(cr_network_unittest
    cr_common
    cr_network
    ${Boost_COROUTINE_LIBRARY} 
    ${Boost_CONTEXT_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY} 
    ${Boost_REGEX_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_THREAD_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Protobuf_LIBRARY}
)
add_test(cr_network_unittest cr_network_unittest)