# - Find CppNetLib library
# Find the native CppNetLib includes and library
#
# CPPNETLIB_INCLUDE_DIR - where to find boost/network/version.hpp, etc.
# CPPNETLIB_LIBRARIES - List of libraries when using boost/network/version.hpp.
# CPPNETLIB_FOUND - True if cpp-netlib found.

find_path(CPPNETLIB_INCLUDE_DIR
  NAMES boost/network/version.hpp
  HINTS ${CPPNETLIB_ROOT_DIR}/include ${CMAKE_INCLUDE_PATH})

find_library(CPPNETLIB_CLIENT_LIBRARY
  NAMES libcppnetlib-client-connections.a cppnetlib-client-connections
  HINTS ${CPPNETLIB_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
  
find_library(CPPNETLIB_SERVER_LIBRARY
  NAMES libcppnetlib-server-parsers.a cppnetlib-server-parsers
  HINTS ${CPPNETLIB_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
  
find_library(CPPNETLIB_URI_LIBRARY
  NAMES libnetwork-uri.a network-uri
  HINTS ${CPPNETLIB_ROOT_DIR}/lib ${CMAKE_LIBRARY_PATH})
  
set(CPPNETLIB_INCLUDE_DIRS ${CPPNETLIB_INCLUDE_DIR})
set(CPPNETLIB_LIBRARIES ${CPPNETLIB_CLIENT_LIBRARY} ${CPPNETLIB_SERVER_LIBRARY} ${CPPNETLIB_URI_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CppNetLib DEFAULT_MSG CPPNETLIB_LIBRARIES CPPNETLIB_INCLUDE_DIRS)

mark_as_advanced(CPPNETLIB_LIBRARIES CPPNETLIB_INCLUDE_DIRS)