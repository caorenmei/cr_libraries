cmake_minimum_required(VERSION 2.6)

project(lua C)

set(_CR_SOPUIRCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lua-5.3.4/src")

set(_CR_SOURCE
    ${_CR_SOPUIRCE_DIR}/lapi.c
    ${_CR_SOPUIRCE_DIR}/lapi.h
    ${_CR_SOPUIRCE_DIR}/lauxlib.c
    ${_CR_SOPUIRCE_DIR}/lauxlib.h
    ${_CR_SOPUIRCE_DIR}/lbaselib.c
    ${_CR_SOPUIRCE_DIR}/lbitlib.c
    ${_CR_SOPUIRCE_DIR}/lcode.c
    ${_CR_SOPUIRCE_DIR}/lcode.h
    ${_CR_SOPUIRCE_DIR}/lcorolib.c
    ${_CR_SOPUIRCE_DIR}/lctype.c
    ${_CR_SOPUIRCE_DIR}/lctype.h
    ${_CR_SOPUIRCE_DIR}/ldblib.c
    ${_CR_SOPUIRCE_DIR}/ldebug.c
    ${_CR_SOPUIRCE_DIR}/ldebug.h
    ${_CR_SOPUIRCE_DIR}/ldo.c
    ${_CR_SOPUIRCE_DIR}/ldo.h
    ${_CR_SOPUIRCE_DIR}/ldump.c
    ${_CR_SOPUIRCE_DIR}/lfunc.c
    ${_CR_SOPUIRCE_DIR}/lfunc.h
    ${_CR_SOPUIRCE_DIR}/lgc.c
    ${_CR_SOPUIRCE_DIR}/lgc.h
    ${_CR_SOPUIRCE_DIR}/linit.c
    ${_CR_SOPUIRCE_DIR}/liolib.c
    ${_CR_SOPUIRCE_DIR}/llex.c
    ${_CR_SOPUIRCE_DIR}/llex.h
    ${_CR_SOPUIRCE_DIR}/llimits.h
    ${_CR_SOPUIRCE_DIR}/lmathlib.c
    ${_CR_SOPUIRCE_DIR}/lmem.c
    ${_CR_SOPUIRCE_DIR}/lmem.h
    ${_CR_SOPUIRCE_DIR}/loadlib.c
    ${_CR_SOPUIRCE_DIR}/lobject.c
    ${_CR_SOPUIRCE_DIR}/lobject.h
    ${_CR_SOPUIRCE_DIR}/lopcodes.c
    ${_CR_SOPUIRCE_DIR}/lopcodes.h
    ${_CR_SOPUIRCE_DIR}/loslib.c
    ${_CR_SOPUIRCE_DIR}/lparser.c
    ${_CR_SOPUIRCE_DIR}/lparser.h
    ${_CR_SOPUIRCE_DIR}/lprefix.h
    ${_CR_SOPUIRCE_DIR}/lstate.c
    ${_CR_SOPUIRCE_DIR}/lstate.h
    ${_CR_SOPUIRCE_DIR}/lstring.c
    ${_CR_SOPUIRCE_DIR}/lstring.h
    ${_CR_SOPUIRCE_DIR}/lstrlib.c
    ${_CR_SOPUIRCE_DIR}/ltable.c
    ${_CR_SOPUIRCE_DIR}/ltable.h
    ${_CR_SOPUIRCE_DIR}/ltablib.c
    ${_CR_SOPUIRCE_DIR}/ltm.c
    ${_CR_SOPUIRCE_DIR}/ltm.h
    ${_CR_SOPUIRCE_DIR}/lua.h
	${_CR_SOPUIRCE_DIR}/lua.hpp
    ${_CR_SOPUIRCE_DIR}/luaconf.h
    ${_CR_SOPUIRCE_DIR}/lualib.h
    ${_CR_SOPUIRCE_DIR}/lundump.c
    ${_CR_SOPUIRCE_DIR}/lundump.h
    ${_CR_SOPUIRCE_DIR}/lutf8lib.c
    ${_CR_SOPUIRCE_DIR}/lvm.c
    ${_CR_SOPUIRCE_DIR}/lvm.h
    ${_CR_SOPUIRCE_DIR}/lzio.c
    ${_CR_SOPUIRCE_DIR}/lzio.h
)

set(_CR_INCLUDES
    ${_CR_SOPUIRCE_DIR}/lauxlib.h
    ${_CR_SOPUIRCE_DIR}/lua.h
	${_CR_SOPUIRCE_DIR}/lua.hpp
    ${_CR_SOPUIRCE_DIR}/luaconf.h
    ${_CR_SOPUIRCE_DIR}/lualib.h
)

if(NOT MSVC)
    find_library(_CR_MATHLIB "m")
endif()

if (MSVC AND lua_MSVC_STATIC_RUNTIME)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
endif (MSVC AND lua_MSVC_STATIC_RUNTIME)

# lib
add_library(lua53 ${_CR_SOURCE})

# lua
add_executable(lua ${_CR_SOPUIRCE_DIR}/lua.c)
target_link_libraries(lua lua53 ${_CR_MATHLIB})

# luac
add_executable(luac ${_CR_SOPUIRCE_DIR}/luac.c)
target_link_libraries(luac lua53 ${_CR_MATHLIB})

# install 
install(FILES ${_CR_INCLUDES} DESTINATION include/lua )
install(FILES ${_CR_INCLUDES} DESTINATION include)
install(TARGETS lua53 lua luac
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)