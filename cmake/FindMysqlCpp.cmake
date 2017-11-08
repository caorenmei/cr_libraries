# - Find MysqlCpp library
# Find the native MysqlCpp includes and library
#
# MYSQLCPP_INCLUDE_DIR - where to find mysqlcppconn-static/db.h, etc.
# MYSQLCPP_LIBRARIES - List of libraries when using mysqlcppconn-static/db.h.
# MYSQLCPP_FOUND - True if mysql-connector-cpp found.

find_path(MYSQLCPP_INCLUDE_DIR
  NAMES mysql_connection.h
  HINTS ${MYSQLCPP_ROOT_DIR}/include ${CMAKE_INCLUDE_PATH})

find_library(MYSQLCPP_LIBRARY
  NAMES libmysqlcppconn-static.a mysqlcppconn-static
  HINTS ${MYSQLCPP_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
  
set(MYSQLCPP_INCLUDE_DIRS ${MYSQLCPP_INCLUDE_DIR})
set(MYSQLCPP_LIBRARIES ${MYSQLCPP_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mysqlcppconn-static DEFAULT_MSG MYSQLCPP_LIBRARIES MYSQLCPP_INCLUDE_DIRS)

mark_as_advanced(MYSQLCPP_LIBRARIES MYSQLCPP_INCLUDE_DIRS)