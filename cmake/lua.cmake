
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_LUA_SRC "${_CR_PROJECT_ROOT}/third_party/lua/cmake")

include(common.cmake)

 # Makefile
if(_CR_MSVC EQUAL 1)
    set(_CR_MAKEFILE "NMake Makefiles")
    set(_CR_MAKE "nmake")
else()
    set(_CR_MAKEFILE "Unix Makefiles")
    set(_CR_MAKE "make")
endif()

# 编译Bson
function(_build_lua buildMode)
    # 工作目录
    _cr_build_path(_CR_LUA_BUILD_DIR lua ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_LUA_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_LUA_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_LUA_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_LUA_INSTALL_DIR lua ${buildMode})
    if(NOT EXISTS "${_CR_LUA_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_LUA_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_LUA_INSTALL_DIR}")
    endif()
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_LUA_INSTALL_DIR} -B "${_CR_LUA_BUILD_DIR}" "${_CR_LUA_SRC}"
            WORKING_DIRECTORY "${_CR_LUA_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(
        COMMAND ${_CR_MAKE} WORKING_DIRECTORY "${_CR_LUA_BUILD_DIR}"
        COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_LUA_BUILD_DIR}"
    )
endfunction()

_build_lua("Release")
_build_lua("Debug")
