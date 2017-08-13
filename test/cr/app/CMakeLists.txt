
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost REQUIRED atomic chrono date_time filesystem log log_setup regex system thread unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_app_unittest_SRCS
    unittest_main.cpp
    application_unittest.cpp
    calc_service.h
    calc_service.cpp
    req_service.h
    req_service.cpp
)

set(cr_app_unittest_PROTOS 
    unittest.proto
)
source_group("Proto Files" FILES ${cr_app_unittest_PROTOS})

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${cr_app_unittest_PROTOS})  
  
add_executable(cr_app_unittest 
    ${cr_app_unittest_SRCS}
    ${cr_app_unittest_PROTOS}
    ${PROTO_SRCS} ${PROTO_HDRS}
)
install(TARGETS cr_app_unittest DESTINATION bin)
target_link_libraries(cr_app_unittest 
    cr_app
    cr_common
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
    ${Boost_LOG_LIBRARY}
    ${Boost_ATOMIC_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${WIN_PSAPI_LIB}
)
add_test(cr_app_unittest cr_app_unittest)