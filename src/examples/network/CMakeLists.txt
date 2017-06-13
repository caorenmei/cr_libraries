
find_package(Boost REQUIRED chrono context coroutine date_time regex system thread timer)
include_directories(${Boost_INCLUDE_DIRS})

# assert.h
add_executable(cr_network_example_pipe pipe_example.cpp)
install(TARGETS cr_network_example_pipe DESTINATION bin)
target_link_libraries(cr_network_example_pipe 
    ${Boost_CHRONO_LIBRARY}
    ${Boost_CONTEXT_LIBRARY}
    ${Boost_COROUTINE_LIBRARY} 
    ${Boost_DATE_TIME_LIBRARY} 
    ${Boost_REGEX_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_THREAD_LIBRARY}
    ${Boost_TIMER_LIBRARY}
    cr_common
    cr_network
    ${_CR_PTHREAD_LIBRARY}
)