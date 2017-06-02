cmake_minimum_required(VERSION 2.6)

project(lua C)

set(_CR_SOPUIRCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../lua-5.3.4")

set(_CR_SOURCE
    ${_CR_SOPUIRCE_DIR}/src/lapi.c
    ${_CR_SOPUIRCE_DIR}/src/lapi.h
    ${_CR_SOPUIRCE_DIR}/src/lauxlib.c
    ${_CR_SOPUIRCE_DIR}/src/lauxlib.h
    ${_CR_SOPUIRCE_DIR}/src/lbaselib.c
    ${_CR_SOPUIRCE_DIR}/src/lbitlib.c
    ${_CR_SOPUIRCE_DIR}/src/lcode.c
    ${_CR_SOPUIRCE_DIR}/src/lcode.h
    ${_CR_SOPUIRCE_DIR}/src/lcorolib.c
    ${_CR_SOPUIRCE_DIR}/src/lctype.c
    ${_CR_SOPUIRCE_DIR}/src/lctype.h
    ${_CR_SOPUIRCE_DIR}/src/ldblib.c
    ${_CR_SOPUIRCE_DIR}/src/ldebug.c
    ${_CR_SOPUIRCE_DIR}/src/ldebug.h
    ${_CR_SOPUIRCE_DIR}/src/ldo.c
    ${_CR_SOPUIRCE_DIR}/src/ldo.h
    ${_CR_SOPUIRCE_DIR}/src/ldump.c
    ${_CR_SOPUIRCE_DIR}/src/lfunc.c
    ${_CR_SOPUIRCE_DIR}/src/lfunc.h
    ${_CR_SOPUIRCE_DIR}/src/lgc.c
    ${_CR_SOPUIRCE_DIR}/src/lgc.h
    ${_CR_SOPUIRCE_DIR}/src/linit.c
    ${_CR_SOPUIRCE_DIR}/src/liolib.c
    ${_CR_SOPUIRCE_DIR}/src/llex.c
    ${_CR_SOPUIRCE_DIR}/src/llex.h
    ${_CR_SOPUIRCE_DIR}/src/llimits.h
    ${_CR_SOPUIRCE_DIR}/src/lmathlib.c
    ${_CR_SOPUIRCE_DIR}/src/lmem.c
    ${_CR_SOPUIRCE_DIR}/src/lmem.h
    ${_CR_SOPUIRCE_DIR}/src/loadlib.c
    ${_CR_SOPUIRCE_DIR}/src/lobject.c
    ${_CR_SOPUIRCE_DIR}/src/lobject.h
    ${_CR_SOPUIRCE_DIR}/src/lopcodes.c
    ${_CR_SOPUIRCE_DIR}/src/lopcodes.h
    ${_CR_SOPUIRCE_DIR}/src/loslib.c
    ${_CR_SOPUIRCE_DIR}/src/lparser.c
    ${_CR_SOPUIRCE_DIR}/src/lparser.h
    ${_CR_SOPUIRCE_DIR}/src/lprefix.h
    ${_CR_SOPUIRCE_DIR}/src/lstate.c
    ${_CR_SOPUIRCE_DIR}/src/lstate.h
    ${_CR_SOPUIRCE_DIR}/src/lstring.c
    ${_CR_SOPUIRCE_DIR}/src/lstring.h
    ${_CR_SOPUIRCE_DIR}/src/lstrlib.c
    ${_CR_SOPUIRCE_DIR}/src/ltable.c
    ${_CR_SOPUIRCE_DIR}/src/ltable.h
    ${_CR_SOPUIRCE_DIR}/src/ltablib.c
    ${_CR_SOPUIRCE_DIR}/src/ltm.c
    ${_CR_SOPUIRCE_DIR}/src/ltm.h
    ${_CR_SOPUIRCE_DIR}/src/lua.h
    ${_CR_SOPUIRCE_DIR}/src/lua.hpp
    ${_CR_SOPUIRCE_DIR}/src/luaconf.h
    ${_CR_SOPUIRCE_DIR}/src/lualib.h
    ${_CR_SOPUIRCE_DIR}/src/lundump.c
    ${_CR_SOPUIRCE_DIR}/src/lundump.h
    ${_CR_SOPUIRCE_DIR}/src/lutf8lib.c
    ${_CR_SOPUIRCE_DIR}/src/lvm.c
    ${_CR_SOPUIRCE_DIR}/src/lvm.h
    ${_CR_SOPUIRCE_DIR}/src/lzio.c
    ${_CR_SOPUIRCE_DIR}/src/lzio.h
)

set(_CR_INCLUDES
    ${_CR_SOPUIRCE_DIR}/src/lauxlib.h
    ${_CR_SOPUIRCE_DIR}/src/lua.h
    ${_CR_SOPUIRCE_DIR}/src/lua.hpp
    ${_CR_SOPUIRCE_DIR}/src/luaconf.h
    ${_CR_SOPUIRCE_DIR}/src/lualib.h
)

# lib
add_library(lua53 ${_CR_SOURCE})

# lua
add_executable(lua ${_CR_SOPUIRCE_DIR}/src/lua.c)
target_link_libraries(lua lua53)

# luac
add_executable(luac ${_CR_SOPUIRCE_DIR}/src/luac.c)
target_link_libraries(luac lua53)

# install 
install(FILES ${_CR_INCLUDES} DESTINATION include/lua )
install(TARGETS lua53 lua luac
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
