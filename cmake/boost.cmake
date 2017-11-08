﻿set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_BOOST_SRC_FLODER "${_CR_PROJECT_ROOT}/third_party/boost/boost_1_64_0")

include(common.cmake)

function(_set_scripts_perm)
    execute_process(COMMAND chmod a+x "${_CR_BOOST_SRC_FLODER}/bootstrap.sh" WORKING_DIRECTORY "${_CR_BOOST_SRC_FLODER}")
    file(GLOB_RECURSE _CR_SHELL_SCRIPTS "${_CR_BOOST_SRC_FLODER}/tools/build/*.sh")
    foreach(scriptFile ${_CR_SHELL_SCRIPTS})
        execute_process(COMMAND chmod a+x "${scriptFile}" WORKING_DIRECTORY "${_CR_BOOST_SRC_FLODER}")
    endforeach()
endfunction()

if(_CR_MSVC EQUAL 1)
    set(_CR_BOOST_BOOTSTRAP "${_CR_BOOST_SRC_FLODER}/bootstrap.bat")
    set(_CR_BOOST_B2_EXE "${_CR_BOOST_SRC_FLODER}/b2.exe")
else()
    _set_scripts_perm()
    set(_CR_BOOST_BOOTSTRAP "${_CR_BOOST_SRC_FLODER}/bootstrap.sh")
    set(_CR_BOOST_B2_EXE "${_CR_BOOST_SRC_FLODER}/b2")
endif()

# 编译b2
if(NOT EXISTS "${_CR_BOOST_B2_EXE}")
     execute_process(COMMAND "${_CR_BOOST_BOOTSTRAP}" WORKING_DIRECTORY "${_CR_BOOST_SRC_FLODER}")
endif()

function(_build_boost buildMode)
    # debug/release
    if("${buildMode}" STREQUAL "Debug")
        set(_CR_BOOST_VARIANT "debug")
    else()
        set(_CR_BOOST_VARIANT "release")
    endif()
    # 编译目录
    _cr_build_path(_CR_BOOST_BUILD_DIR "boost" ${buildMode})
    if(NOT EXISTS "${_CR_BOOST_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_BOOST_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_BOOST_BUILD_DIR}")
    endif()
    # 安装目录
     _cr_install_path(_CR_BOOST_INSTALL_DIR "boost" ${buildMode})
    if(NOT EXISTS "${_CR_BOOST_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_BOOST_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_BOOST_INSTALL_DIR}")
    endif()
    set(_CR_BOOST_STAGE_LIBS "${_CR_BOOST_INSTALL_DIR}/lib")
    execute_process(
        COMMAND "${_CR_BOOST_B2_EXE}" --stagedir=${_CR_BOOST_INSTALL_DIR} --build-dir=${_CR_BOOST_BUILD_DIR} variant=${_CR_BOOST_VARIANT} link=static threading=multi runtime-link=static 
        WORKING_DIRECTORY "${_CR_BOOST_SRC_FLODER}"
    )
    # boost头文件目录
    set(_CR_BOOST_INCLUDE_DIR "${_CR_BOOST_INSTALL_DIR}/include")
    if(NOT EXISTS "${_CR_BOOST_INCLUDE_DIR}")
        file(MAKE_DIRECTORY "${_CR_BOOST_INCLUDE_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_BOOST_INCLUDE_DIR}")
    endif()
    file(COPY "${_CR_BOOST_SRC_FLODER}/boost" DESTINATION "${_CR_BOOST_INCLUDE_DIR}/")
endfunction()

_build_boost("Debug")
_build_boost("Release")