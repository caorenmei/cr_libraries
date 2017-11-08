# - Find Xlnt library
# Find the native Xlnt includes and library
#
# XLNT_INCLUDE_DIR - where to find xlnt/xlnt.hpp, etc.
# XLNT_LIBRARIES - List of libraries when using xlnt/xlnt.hpp.
# XLNT_FOUND - True if jemalloc found.

find_path(XLNT_INCLUDE_DIR
  NAMES xlnt/xlnt.hpp
  HINTS ${XLNT_ROOT_DIR}/include ${CMAKE_INCLUDE_PATH})

find_library(XLNT_LIBRARY
  NAMES libxlnt.a xlnt
  HINTS ${XLNT_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
  
set(XLNT_INCLUDE_DIRS ${XLNT_INCLUDE_DIR})
set(XLNT_LIBRARIES ${XLNT_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(xlnt DEFAULT_MSG XLNT_LIBRARIES XLNT_INCLUDE_DIRS)

mark_as_advanced(XLNT_LIBRARIES XLNT_INCLUDE_DIRS)