# - Find Selene library
# Find the native Selene includes and library
#
# SELENE_INCLUDE_DIR - where to find selene.h, etc.
# SELENE_LIBRARIES - List of libraries when using selene.h.
# SELENE_FOUND - True if selene found.

find_path(SELENE_INCLUDE_DIR
  NAMES selene.h
  HINTS ${SELENE_ROOT_DIR}/include ${CMAKE_INCLUDE_PATH})
  
set(SELENE_INCLUDE_DIRS ${SELENE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(seleneclient DEFAULT_MSG SELENE_INCLUDE_DIRS)

mark_as_advanced(SELENE_INCLUDE_DIRS)