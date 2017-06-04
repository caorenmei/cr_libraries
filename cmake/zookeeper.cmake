
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_ZOOKEEPER_CMAKE "${_CR_PROJECT_ROOT}/third_party/zookeeper/cmake")
set(_CR_ZOOKEEPER_SRC "${_CR_PROJECT_ROOT}/third_party/zookeeper/zookeeper-3.4.10/src/c")

include(common.cmake)

 # Makefile
if(_CR_MSVC EQUAL 1)
    set(_CR_MAKEFILE "NMake Makefiles")
    set(_CR_MAKE "nmake")
else()
    set(_CR_MAKEFILE "Unix Makefiles")
    set(_CR_MAKE "make")
endif()

# windows下编译
function(_build_zookeeper_win buildMode)
    # 工作目录
    _cr_build_path(_CR_ZOOKEEPER_BUILD_DIR "zookeeper" ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_ZOOKEEPER_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_ZOOKEEPER_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_ZOOKEEPER_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_ZOOKEEPER_INSTALL_DIR "zookeeper" ${buildMode})
    if(NOT EXISTS "${_CR_ZOOKEEPER_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_ZOOKEEPER_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_ZOOKEEPER_INSTALL_DIR}")
    endif()
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_ZOOKEEPER_INSTALL_DIR} -B "${_CR_ZOOKEEPER_BUILD_DIR}" "${_CR_ZOOKEEPER_CMAKE}"
            WORKING_DIRECTORY "${_CR_ZOOKEEPER_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(
        COMMAND ${_CR_MAKE} WORKING_DIRECTORY "${_CR_ZOOKEEPER_BUILD_DIR}"
        COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_ZOOKEEPER_BUILD_DIR}"
    )
endfunction()

# unix下编译
function(_build_zookeeper_unix buildMode)
    _cr_install_path(_CR_ZOOKEEPER_INSTALL_DIR "zookeeper" ${buildMode})
    execute_process(COMMAND pwd OUTPUT_VARIABLE _CR_ZOOKEEPER_INSTALL_DIR WORKING_DIRECTORY "${_CR_ZOOKEEPER_INSTALL_DIR}")
    execute_process(COMMAND chmod a+x "${_CR_ZOOKEEPER_SRC}/configure" WORKING_DIRECTORY "${_CR_ZOOKEEPER_SRC}")
    execute_process(COMMAND ${_CR_ZOOKEEPER_SRC}/configure --prefix="${_CR_ZOOKEEPER_INSTALL_DIR}"  WORKING_DIRECTORY "${_CR_ZOOKEEPER_SRC}")
    execute_process(COMMAND ${_CR_MAKE} WORKING_DIRECTORY "${_CR_ZOOKEEPER_SRC}")
    execute_process(COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_ZOOKEEPER_SRC}")
endfunction()

function(_build_zookeeper buildMode)
    if(_CR_MSVC EQUAL 1)
        _build_zookeeper_win(${buildMode})
    else()
        _build_zookeeper_unix(${buildMode})
    endif()
endfunction()

_build_zookeeper("Release")
_build_zookeeper("Debug")
