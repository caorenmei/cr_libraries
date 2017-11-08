set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_XLNT_ROOT "${_CR_PROJECT_ROOT}/third_party/xlnt")

# Makefile
if(_CR_MSVC EQUAL 1)
    set(_CR_MAKEFILE "NMake Makefiles")
    set(_CR_MAKE "nmake")
    set(_CR_XLNT_LIB "xlnt.lib")
else()
    set(_CR_MAKEFILE "Unix Makefiles")
    set(_CR_MAKE "make")
	set(_CR_XLNT_LIB "libxlnt.a")
endif()

include(common.cmake)

function(_build_xlnt buildMode)
    # 工作目录
    _cr_build_path(_CR_XLNT_BUILD_DIR "xlnt" ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_XLNT_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_XLNT_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_XLNT_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_XLNT_INSTALL_DIR "xlnt" ${buildMode})
    if(NOT EXISTS "${_CR_XLNT_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_XLNT_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_XLNT_INSTALL_DIR}")
    endif()
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" -DCMAKE_BUILD_TYPE=${buildMode} -DSTATIC=ON -DTESTS=OFF -DCMAKE_INSTALL_PREFIX=${_CR_XLNT_INSTALL_DIR} -Dxlnt_MSVC_STATIC_RUNTIME=ON -B ${_CR_XLNT_BUILD_DIR} ${_CR_XLNT_ROOT} 
            WORKING_DIRECTORY "${_CR_XLNT_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_XLNT_BUILD_DIR}")
endfunction()

_build_xlnt("Debug")
_build_xlnt("Release")