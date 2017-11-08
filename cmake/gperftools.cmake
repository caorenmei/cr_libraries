set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_GPERFTOOLS_SRC "${_CR_PROJECT_ROOT}/third_party/gperftools")

include(common.cmake)

# 编译Bson
function(_build_gperftools buildMode)
    # 工作目录
    _cr_build_path(_CR_GPERFTOOLS_BUILD_DIR gperftools ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_GPERFTOOLS_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_GPERFTOOLS_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_GPERFTOOLS_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_GPERFTOOLS_INSTALL_DIR gperftools ${buildMode})
    if(NOT EXISTS "${_CR_GPERFTOOLS_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_GPERFTOOLS_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_GPERFTOOLS_INSTALL_DIR}")
    endif()
    # 拷贝工作目录
    file(COPY "${_CR_GPERFTOOLS_SRC}/" DESTINATION "${_CR_GPERFTOOLS_BUILD_DIR}/")
    # 生成配置
    execute_process(COMMAND chmod a+x autogen.sh Makefile.am WORKING_DIRECTORY ${_CR_GPERFTOOLS_BUILD_DIR})
    execute_process(COMMAND ./autogen.sh WORKING_DIRECTORY ${_CR_GPERFTOOLS_BUILD_DIR})
    execute_process(COMMAND ./configure --prefix=${_CR_GPERFTOOLS_INSTALL_DIR} --enable-minimal WORKING_DIRECTORY ${_CR_GPERFTOOLS_BUILD_DIR})
    # 生成安装
    execute_process(COMMAND make install WORKING_DIRECTORY ${_CR_GPERFTOOLS_BUILD_DIR})
endfunction()

_build_gperftools("Debug")
_build_gperftools("Release")