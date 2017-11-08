# - Find uuid library
# Find the native uuid includes and library
#
# UUID_INCLUDE_DIR - where to find uuid/uuid.h, etc.
# UUID_LIBRARIES - List of libraries when using uuid/uuid.h.
# UUID_FOUND - True if jemalloc found.

find_path(UUID_INCLUDE_DIR
  NAMES uuid/uuid.h
  HINTS ${UUID_ROOT_DIR}/include ${CMAKE_INCLUDE_PATH})

find_library(UUID_LIBRARY
  NAMES libuuid.a uuid
  HINTS ${UUID_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
  
set(UUID_INCLUDE_DIRS ${UUID_INCLUDE_DIR})
set(UUID_LIBRARIES ${UUID_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(uuid DEFAULT_MSG UUID_LIBRARIES UUID_INCLUDE_DIRS)

mark_as_advanced(UUID_LIBRARIES UUID_INCLUDE_DIRS)