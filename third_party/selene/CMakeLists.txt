cmake_minimum_required(VERSION 2.8)
project(Selene)

enable_testing()

if(WIN32)
    set(ENV_TERMINAL ";")
else()
    set(ENV_TERMINAL ":")
endif()
if (LUA_DIR)
    set(ENV{PATH} "${LUA_DIR}/bin${ENV_TERMINAL}$ENV{PATH}")
    set(CMAKE_INCLUDE_PATH "${LUA_DIR}/include ${CMAKE_INCLUDE_PATH}")
    set(CMAKE_LIBRARY_PATH "${LUA_DIR}/lib ${CMAKE_LIBRARY_PATH}")
endif()

if (MSVC AND selene_MSVC_STATIC_RUNTIME)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
endif (MSVC AND selene_MSVC_STATIC_RUNTIME)

find_package(Lua REQUIRED)
if(NOT LUA_INCLUDE_DIR)
    message(SEND_ERROR "LUA_INCLUDE_DIR is required if no PkgConfig is available!")
endif()
if(NOT LUA_LIBRARIES)
    message(SEND_ERROR "LUA_LIBRARIES is required if no PkgConfig is available!")
endif()

if(${LUA_INCLUDE_DIR})
  if(NOT EXISTS ${LUA_INCLUDE_DIR}/lua.h)
    message(SEND_ERROR "Can't find lua.h in ${LUA_INCLUDE_DIR}")
  endif()
endif()

include_directories(${LUA_INCLUDE_DIR})

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -stdlib=libc++")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")
elseif(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG /bigobj")
endif(APPLE)

include_directories(include)
file(GLOB headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  include/*.h include/selene/*.h)

add_executable(test_runner ${CMAKE_CURRENT_SOURCE_DIR}/test/Test.cpp)
install(TARGETS test_runner DESTINATION bin)
target_link_libraries(test_runner ${LUA_LIBRARIES})
add_test(selene_test test_runner)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)