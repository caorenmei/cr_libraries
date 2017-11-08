# - Find Mysql library
# Find the native Mysql includes and library
#
# MYSQL_INCLUDE_DIR - where to find mysqlclient/db.h, etc.
# MYSQL_LIBRARIES - List of libraries when using mysqlclient/db.h.
# MYSQL_FOUND - True if mysql found.

find_path(MYSQL_INCLUDE_DIR
  NAMES mysql.h
  HINTS ${MYSQL_ROOT_DIR}/include ${CMAKE_INCLUDE_PATH})

find_library(MYSQL_LIBRARY
  NAMES libmysqlclient_r.a mysqlclient_r mysqlclient
  HINTS ${MYSQL_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
  
set(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})
set(MYSQL_LIBRARIES ${MYSQL_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mysqlclient DEFAULT_MSG MYSQL_LIBRARIES MYSQL_INCLUDE_DIRS)

mark_as_advanced(MYSQL_LIBRARIES MYSQL_INCLUDE_DIRS)