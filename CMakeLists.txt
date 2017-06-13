cmake_minimum_required(VERSION 2.8)

project (cr_libraris CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSVC)
    set(_CR_ENV_TERMINAL ";")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /EHs /DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /GS /Od /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL /Gy /O2 -DNDEBUG")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /DEBUG")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WIN32_WINNT=0x0501)
else()
    set(_CR_ENV_TERMINAL ":")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -g -lpthread")
endif()

add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)

# 第三方库
foreach(libName boost cpp-netlib eluna lua mongodb-bson mongodb-driver mongodb-driver-cxx mysql mysql-connector-cpp protobuf rapidxml zookeeper)
    set(ENV{PATH} "${PROJECT_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/${libName}/bin${_CR_ENV_TERMINAL}$ENV{PATH}")
    set(ENV{CMAKE_INCLUDE_PATH} "${PROJECT_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/include${_CR_ENV_TERMINAL}$ENV{CMAKE_INCLUDE_PATH}")
    set(ENV{CMAKE_LIBRARY_PATH} "${PROJECT_SOURCE_DIR}/install/${CMAKE_BUILD_TYPE}/lib${_CR_ENV_TERMINAL}$ENV{CMAKE_LIBRARY_PATH}")
endforeach()

# Boost
option(Boost_USE_STATIC_LIBS "Use static libs" ON)

# 子文件
add_subdirectory(src)