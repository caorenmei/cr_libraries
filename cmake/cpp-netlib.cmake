
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_CPP_NETLIB_SRC "${_CR_PROJECT_ROOT}/third_party/cpp-netlib/cpp-netlib-0.12.0-final")

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
function(_build_cpp_netlib buildMode)
    # 工作目录
    _cr_build_path(_CR_CPP_NETLIB_BUILD_DIR "cpp-netlib" ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_CPP_NETLIB_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_CPP_NETLIB_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_CPP_NETLIB_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_CPP_NETLIB_INSTALL_DIR "cpp-netlib" ${buildMode})
    if(NOT EXISTS "${_CR_CPP_NETLIB_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_CPP_NETLIB_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_CPP_NETLIB_INSTALL_DIR}")
    endif()
    _cr_install_path(ENV{BOOST_ROOT} "boost" ${buildMode})
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" -DCPP-NETLIB_BUILD_TESTS=OFF -DCPP-NETLIB_BUILD_EXAMPLES=OFF -DCPP-NETLIB_ENABLE_HTTPS=OFF -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_CPP_NETLIB_INSTALL_DIR} -Dcppnetlib_MSVC_STATIC_RUNTIME=ON -B "${_CR_CPP_NETLIB_BUILD_DIR}" "${_CR_CPP_NETLIB_SRC}"
            WORKING_DIRECTORY "${_CR_CPP_NETLIB_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_CPP_NETLIB_BUILD_DIR}")
endfunction()

_build_cpp_netlib("Debug")
_build_cpp_netlib("Release")
