
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(cr_logging_SRCS
    logging.h
    logging.cpp
)

set(cr_logging_HEADERS
    logging.h
)
 
add_library(cr_log ${cr_logging_SRCS})
install(FILES ${cr_logging_HEADERS} DESTINATION include/cr/log)
install(TARGETS cr_log DESTINATION lib)