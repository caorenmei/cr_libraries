cmake_minimum_required(VERSION 2.6)

project(lua C)

set(_CR_SOPUIRCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../zookeeper-3.4.10")

set(_CR_SOURCE
    ${_CR_SOPUIRCE_DIR}/src/c/src/zk_adaptor.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/zk_hashtable.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/zk_hashtable.c
    ${_CR_SOPUIRCE_DIR}/src/c/generated/zookeeper.jute.h
    ${_CR_SOPUIRCE_DIR}/src/c/generated/zookeeper.jute.c
    ${_CR_SOPUIRCE_DIR}/src/c/src/hashtable/hashtable.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/hashtable/hashtable.c
    ${_CR_SOPUIRCE_DIR}/src/c/src/hashtable/hashtable_itr.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/hashtable/hashtable_itr.c
    ${_CR_SOPUIRCE_DIR}/src/c/src/hashtable/hashtable_private.h
    ${_CR_SOPUIRCE_DIR}/src/c/include/proto.h
    ${_CR_SOPUIRCE_DIR}/src/c/include/recordio.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/recordio.c
    ${_CR_SOPUIRCE_DIR}/src/c/include/zookeeper.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/zookeeper.c
    ${_CR_SOPUIRCE_DIR}/src/c/include/zookeeper_log.h
    ${_CR_SOPUIRCE_DIR}/src/c/include/zookeeper_version.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/zk_log.c
    ${_CR_SOPUIRCE_DIR}/src/c/src/mt_adaptor.c  
    ${_CR_SOPUIRCE_DIR}/src/c/include/winconfig.h
    ${_CR_SOPUIRCE_DIR}/src/c/include/winstdint.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/winport.h
    ${_CR_SOPUIRCE_DIR}/src/c/src/winport.c
)

include_directories(${_CR_SOPUIRCE_DIR}/src/c/include)
include_directories(${_CR_SOPUIRCE_DIR}/src/c/generated)
if (MSVC)
    add_definitions(/D "THREADED")
else()
    add_definitions(-DTHREADED)
endif()

# lib
add_library(zookeeper ${_CR_SOURCE})

# install 
install(DIRECTORY ${_CR_SOPUIRCE_DIR}/src/c/include/ DESTINATION include/zookeeper)
install(FILES ${_CR_SOPUIRCE_DIR}/src/c/generated/zookeeper.jute.h DESTINATION include/zookeeper )
install(TARGETS zookeeper
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
