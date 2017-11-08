set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_UUID_SRC "${_CR_PROJECT_ROOT}/third_party/uuid/libuuid-1.0.3")

include(common.cmake)

# 编译Bson
function(_build_uuid buildMode)
    # 工作目录
    _cr_build_path(_CR_UUID_BUILD_DIR uuid ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_UUID_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_UUID_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_UUID_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_UUID_INSTALL_DIR uuid ${buildMode})
    if(NOT EXISTS "${_CR_UUID_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_UUID_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_UUID_INSTALL_DIR}")
    endif()
    # 拷贝工作目录
    file(COPY "${_CR_UUID_SRC}/" DESTINATION "${_CR_UUID_BUILD_DIR}/")
    # 生成配置
    execute_process(COMMAND chmod a+x config.guess config.sub configure depcomp install-sh missing WORKING_DIRECTORY ${_CR_UUID_BUILD_DIR})
    execute_process(COMMAND ./configure --prefix=${_CR_UUID_INSTALL_DIR} WORKING_DIRECTORY ${_CR_UUID_BUILD_DIR})
    # 生成安装
    execute_process(COMMAND make install WORKING_DIRECTORY ${_CR_UUID_BUILD_DIR})
endfunction()

_build_uuid("Debug")
_build_uuid("Release")