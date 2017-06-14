
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_framework_common_SRCS
    path_directory.h
    path_directory.cpp
)

set(cr_framework_common_HEADERS
    path_directory.h
)
 
add_library(cr_framework_common ${cr_framework_common_SRCS})
install(FILES ${cr_framework_common_HEADERS} DESTINATION include/cr/framework/common)
install(TARGETS cr_framework_common DESTINATION lib)