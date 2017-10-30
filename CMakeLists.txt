cmake_minimum_required(VERSION 2.8)

project (cr_libraris CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSVC)
    set(_CR_ENV_TERMINAL ";")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /EHs /DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /GS /Od /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL /Gy /MT /O2 -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /DEBUG")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WIN32_WINNT=0x0601)
else()
    set(_CR_ENV_TERMINAL ":")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -g")
    set(_CR_PTHREAD_LIBRARY "pthread")
endif()

add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)

# 第三方库
foreach(libName boost cpp-netlib lua mysql mysql-connector-cpp protobuf rapidxml rocksdb)
    set(ENV{PATH} "${PROJECT_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/${libName}/bin${_CR_ENV_TERMINAL}$ENV{PATH}")
    set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/include${_CR_ENV_TERMINAL}${CMAKE_INCLUDE_PATH}")
    set(CMAKE_LIBRARY_PATH "${PROJECT_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/lib${_CR_ENV_TERMINAL}${CMAKE_LIBRARY_PATH}")
endforeach()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Boost
option(Boost_USE_STATIC_LIBS "Use static libs" ON)
option(Boost_USE_STATIC_RUNTIME  "Use static runtime" ON)

# 子文件
add_subdirectory(src)