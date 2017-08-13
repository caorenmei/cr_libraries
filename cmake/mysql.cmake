
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_MYSQL_SRC "${_CR_PROJECT_ROOT}/third_party/mysql/mysql-server-5.6")
set(_CR_MYSQL_CONNECTOR_CPP_SRC "${_CR_PROJECT_ROOT}/third_party/mysql/mysql-connector-cpp-1.1")

include(common.cmake)

 # Makefile
if(_CR_MSVC EQUAL 1)
    set(_CR_MAKEFILE "NMake Makefiles")
    set(_CR_MAKE "nmake")
    set(_CR_EXE_LINKER_FLAGS "-DCMAKE_EXE_LINKER_FLAGS=/DEBUG")
else()
    set(_CR_MAKEFILE "Unix Makefiles")
    set(_CR_MAKE "make")
endif()

# 编译Mysql
function(_build_mysql buildMode)
    # 工作目录
    _cr_build_path(_CR_MYSQL_BUILD_DIR "mysql" ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_MYSQL_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_MYSQL_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MYSQL_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_MYSQL_INSTALL_DIR "mysql" ${buildMode})
    if(NOT EXISTS "${_CR_MYSQL_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_MYSQL_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MYSQL_INSTALL_DIR}")
    endif()
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" ${_CR_EXE_LINKER_FLAGS} -DWITHOUT_SERVER=TRUE -DENABLED_PROFILING=OFF -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_MYSQL_INSTALL_DIR} -Dmysql_MSVC_STATIC_RUNTIME=ON -B "${_CR_MYSQL_BUILD_DIR}" "${_CR_MYSQL_SRC}"
            WORKING_DIRECTORY "${_CR_MYSQL_BUILD_DIR}"
    )
    # 编译 & 安装
	execute_process(COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_MYSQL_BUILD_DIR}")
endfunction()

function(_build_mysql_connector_cpp buildMode)
    # 工作目录
    _cr_build_path(_CR_MYSQL_CONNECTOR_CXX_BUILD_DIR "mysql-connector-cpp" ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_MYSQL_CONNECTOR_CXX_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_MYSQL_CONNECTOR_CXX_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MYSQL_CONNECTOR_CXX_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_MYSQL_CONNECTOR_CPP_INSTALL_DIR "mysql-connector-cpp" ${buildMode})
    if(NOT EXISTS "${_CR_MYSQL_CONNECTOR_CPP_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_MYSQL_CONNECTOR_CPP_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MYSQL_CONNECTOR_CPP_INSTALL_DIR}")
    endif()
    _cr_install_path(ENV{BOOST_ROOT} "boost" ${buildMode})
    _cr_install_path(_CR_MYSQL_DIR "mysql" ${buildMode})
    execute_process(COMMAND "${_CR_MYSQL_DIR}/bin/mysql" --version OUTPUT_VARIABLE _CR_MYSQL_VERSION_LONG)
    string(REGEX MATCH "[0-9]+.[0-9].[0-9]+" _CR_MYSQL_VERSION "${_CR_MYSQL_VERSION_LONG}")
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" -DMYSQL_VERSION="${_CR_MYSQL_VERSION}" -DMYSQL_DIR=${_CR_MYSQL_DIR} 
            -DMYSQLCPPCONN_BUILD_EXAMPLES=OFF -DMYSQLCLIENT_STATIC_LINKING=TRUE
            -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_MYSQL_CONNECTOR_CPP_INSTALL_DIR} 
            -B "${_CR_MYSQL_CONNECTOR_CXX_BUILD_DIR}" "${_CR_MYSQL_CONNECTOR_CPP_SRC}"
        WORKING_DIRECTORY "${_CR_MYSQL_CONNECTOR_CXX_BUILD_DIR}"
    )
    # 编译 & 安装
	execute_process(COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_MYSQL_CONNECTOR_CXX_BUILD_DIR}")
endfunction()

_build_mysql("Release")
_build_mysql("Debug")

_build_mysql_connector_cpp("Release")
_build_mysql_connector_cpp("Debug")
