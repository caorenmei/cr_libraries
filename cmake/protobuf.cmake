
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_PROTOBUF_ROOT "${_CR_PROJECT_ROOT}/third_party/protobuf/protobuf-3.3.0/cmake")

# Makefile
if(_CR_MSVC EQUAL 1)
    set(_CR_MAKEFILE "NMake Makefiles")
    set(_CR_MAKE "nmake")
else()
    set(_CR_MAKEFILE "Unix Makefiles")
    set(_CR_MAKE "make")
endif()

include(common.cmake)

function(_build_protobuf buildMode)
    # 工作目录
    _cr_build_path(_CR_PROTOBUF_BUILD_DIR "protobuf" ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_PROTOBUF_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_PROTOBUF_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_PROTOBUF_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_PROTOBUF_INSTALL_DIR "protobuf" ${buildMode})
    if(NOT EXISTS "${_CR_PROTOBUF_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_PROTOBUF_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_PROTOBUF_INSTALL_DIR}")
    endif()
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" -DCMAKE_BUILD_TYPE=${buildMode} -Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_WITH_ZLIB=OFF -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=${_CR_PROTOBUF_INSTALL_DIR} -B ${_CR_PROTOBUF_BUILD_DIR} ${_CR_PROTOBUF_ROOT} 
            WORKING_DIRECTORY "${_CR_PROTOBUF_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(COMMAND ${_CR_MAKE} WORKING_DIRECTORY "${_CR_PROTOBUF_BUILD_DIR}")
	execute_process(COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_PROTOBUF_BUILD_DIR}")
endfunction()

_build_protobuf("Release")
_build_protobuf("Debug")
