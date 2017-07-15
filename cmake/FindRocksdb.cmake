# - Find Rocksdb library
# Find the native Rocksdb includes and library
#
# ROCKSDB_INCLUDE_DIR - where to find rocksdb/db.h, etc.
# ROCKSDB_LIBRARIES - List of libraries when using rocksdb/db.h.
# ROCKSDB_FOUND - True if jemalloc found.

find_path(ROCKSDB_INCLUDE_DIR
  NAMES rocksdb/db.h
  HINTS ${ROCKSDB_ROOT_DIR}/include ${CMAKE_INCLUDE_PATH})

find_library(ROCKSDB_LIBRARY
  NAMES rocksdb
  HINTS ${ROCKSDB_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
  
set(ROCKSDB_INCLUDE_DIRS ${ROCKSDB_INCLUDE_DIR})
set(ROCKSDB_LIBRARIES ${ROCKSDB_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(rocksdb DEFAULT_MSG ROCKSDB_LIBRARIES ROCKSDB_INCLUDE_DIRS)

mark_as_advanced(ROCKSDB_LIBRARIES ROCKSDB_INCLUDE_DIRS)