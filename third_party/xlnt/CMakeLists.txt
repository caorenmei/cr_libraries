cmake_minimum_required(VERSION 3.2)
project(xlnt_all)

# This indicates to CMakeLists in subdirectories that they are part of a larger project
set(COMBINED_PROJECT TRUE)

# Library type
option(STATIC "Set to ON to build xlnt as a static library instead of a shared library" OFF)

# Optional components
option(TESTS "Set to OFF to skip building test executable (in ./tests)" ON)
option(SAMPLES "Set to ON to build executable code samples (in ./samples)" OFF)
option(BENCHMARKS "Set to ON to build performance benchmarks (in ./benchmarks)" OFF)

# Platform specific options
if(NOT MSVC)
    option(COVERAGE "Generate coverage data using gcov" OFF)
endif()

if (MSVC AND xlnt_MSVC_STATIC_RUNTIME)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
endif (MSVC AND xlnt_MSVC_STATIC_RUNTIME)

# Add components according to selected options
if(SAMPLES)
    add_subdirectory(samples)
endif()

if(BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(TESTS)
    add_subdirectory(tests)
endif()

add_subdirectory(source)
