
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_BOOST_SRC_FLODER "${_CR_PROJECT_ROOT}/third_party/boost/boost_1_64_0")
set(_CR_BOOST_STAGE_LIB_FLODER "${_CR_BOOST_SRC_FLODER}/stage/lib")

include(common.cmake)

if(_CR_MSVC EQUAL 1)
    set(_CR_BOOST_BOOTSTRAP "${_CR_BOOST_SRC_FLODER}/bootstrap.bat")
    set(_CR_BOOST_B2_EXE "${_CR_BOOST_SRC_FLODER}/b2.exe")
else()
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
    # 编译
    file(GLOB_RECURSE _CR_BOOST_STAGE_LIBS "${_CR_BOOST_STAGE_LIB_FLODER}/*")
	if(_CR_BOOST_STAGE_LIBS)
		file(REMOVE ${_CR_BOOST_STAGE_LIBS})
	endif()
    execute_process(COMMAND "${_CR_BOOST_B2_EXE}" ${_CR_BOOST_VARIANT} link=static WORKING_DIRECTORY "${_CR_BOOST_SRC_FLODER}")
    # boost头文件目录
    _cr_install_path(_CR_BOOST_INSTALL_PREFIX boost ${buildMode})
    set(_CR_BOOST_INCLUDE_DIR "${_CR_BOOST_INSTALL_PREFIX}/include")
    if(NOT EXISTS "${_CR_BOOST_INCLUDE_DIR}")
        file(MAKE_DIRECTORY "${_CR_BOOST_INCLUDE_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_BOOST_INCLUDE_DIR}")
    endif()
    file(COPY "${_CR_BOOST_SRC_FLODER}/boost" DESTINATION "${_CR_BOOST_INCLUDE_DIR}/")
    message(STATUS "COPY ${_CR_BOOST_SRC_FLODER}/boost DESTINATION ${_CR_BOOST_INCLUDE_DIR}/")
    # libs
    set(_CR_BOOST_LIBRARY_DIR "${_CR_BOOST_INSTALL_PREFIX}/lib")
    if(NOT EXISTS "${_CR_BOOST_LIBRARY_DIR}")
        file(MAKE_DIRECTORY "${_CR_BOOST_LIBRARY_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_BOOST_LIBRARY_DIR}")
    endif()
    file(COPY "${_CR_BOOST_STAGE_LIB_FLODER}/" DESTINATION "${_CR_BOOST_LIBRARY_DIR}")
    message(STATUS "COPY ${_CR_BOOST_SRC_FLODER}/stage/lib/ DESTINATION ${_CR_BOOST_LIBRARY_DIR}")
endfunction()

_build_boost("Debug")
_build_boost("Release")


