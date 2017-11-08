# - Find gperftools library
# Find the native gperftools includes and library
#
# GPERFTOOLS_INCLUDE_DIR - where to find gperftools/tcmalloc.h, etc.
# GPERFTOOLS_LIBRARIES - List of libraries when using gperftools/tcmalloc.h.
# GPERFTOOLS_FOUND - True if gperftools found.

find_path(GPERFTOOLS_INCLUDE_DIR
  NAMES gperftools/tcmalloc.h
  HINTS ${GPERFTOOLS_ROOT_DIR}/include ${CMAKE_INCLUDE_PATH})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_library(GPERFTOOLS_TCMALLOC_MINIMAL_LIBRARY
        NAMES libtcmalloc_minimal_debug.a tcmalloc_minimal_debug
        HINTS ${GPERFTOOLS_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
else()
    find_library(GPERFTOOLS_TCMALLOC_MINIMAL_LIBRARY
        NAMES libtcmalloc_minimal.a tcmalloc_minimal
        HINTS ${GPERFTOOLS_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
endif()
  
set(GPERFTOOLS_INCLUDE_DIRS ${GPERFTOOLS_INCLUDE_DIR})
set(GPERFTOOLS_LIBRARIES ${GPERFTOOLS_TCMALLOC_MINIMAL_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gperftools DEFAULT_MSG GPERFTOOLS_LIBRARIES GPERFTOOLS_INCLUDE_DIRS)

mark_as_advanced(GPERFTOOLS_LIBRARIES GPERFTOOLS_INCLUDE_DIRS)