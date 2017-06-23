
find_package(Boost REQUIRED chrono context coroutine date_time regex system thread timer)
include_directories(${Boost_INCLUDE_DIRS})

# assert.h
add_executable(cr_concurrent_example_pipe pipe_example.cpp)
install(TARGETS cr_concurrent_example_pipe DESTINATION bin)
target_link_libraries(cr_concurrent_example_pipe 
    cr_concurrent
    cr_common
    ${Boost_COROUTINE_LIBRARY} 
    ${Boost_TIMER_LIBRARY}
    ${Boost_CONTEXT_LIBRARY}
    ${Boost_CHRONO_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY} 
    ${Boost_REGEX_LIBRARY} 
    ${Boost_SYSTEM_LIBRARY} 
    ${Boost_THREAD_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)