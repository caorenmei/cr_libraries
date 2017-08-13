find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

set(cr_app_SRCS
    application.h
    application.cpp
    cluster.h
    exceptions.h
    local_cluster.h
    local_cluster.cpp
    message.h
    service.h
    service.cpp
)

set(cr_app_HEADERS
    application.h
    exceptions.h
    cluster.h
    message.h
    service.h
)

add_library(cr_app ${cr_app_SRCS})
install(FILES ${cr_app_HEADERS} DESTINATION include/cr/app)
install(TARGETS cr_app DESTINATION lib)