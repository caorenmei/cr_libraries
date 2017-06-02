
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

set(_CR_MONGODB_ROOT "${_CR_PROJECT_ROOT}/third_party/mongodb")
set(_CR_MONGODB_BSON_SRC "${_CR_MONGODB_ROOT}/libbson-1.6.3")
set(_CR_MONGODB_DRIVER_SRC "${_CR_MONGODB_ROOT}/mongo-c-driver-1.6.3")
set(_CR_MONGODB_DRIVER_CXX_SRC "${_CR_MONGODB_ROOT}/mongo-cxx-driver-r3.1.1")

include(common.cmake)

 # Makefile
if(_CR_MSVC EQUAL 1)
    set(_CR_MAKEFILE "NMake Makefiles")
    set(_CR_MAKE "nmake")
    set(_CR_ENABLE_SSL "-DENABLE_SSL=WINDOWS")
    set(_CR_ENABLE_SASL "-DENABLE_SASL=OFF")
else()
    set(_CR_MAKEFILE "Unix Makefiles")
    set(_CR_MAKE "make")
endif()

# 编译Bson
function(_build_mongodb_bson buildMode)
    # 工作目录
    _cr_build_path(_CR_MONGODB_BSON_BUILD_DIR "mongodb-bson" ${buildMode})
    if(NOT EXISTS "${_CR_MONGODB_BSON_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_MONGODB_BSON_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MONGODB_BSON_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_MONGODB_BSON_INSTALL_DIR "mongodb-bson" ${buildMode})
    if(NOT EXISTS "${_CR_MONGODB_BSON_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_MONGODB_BSON_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MONGODB_BSON_INSTALL_DIR}")
    endif()
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" -DCMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS=TRUE -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_MONGODB_BSON_INSTALL_DIR} -B "${_CR_MONGODB_BSON_BUILD_DIR}" "${_CR_MONGODB_BSON_SRC}"
            WORKING_DIRECTORY "${_CR_MONGODB_BSON_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(
        COMMAND ${_CR_MAKE} WORKING_DIRECTORY "${_CR_MONGODB_BSON_BUILD_DIR}"
    )
	execute_process(
        COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_MONGODB_BSON_BUILD_DIR}"
    )
endfunction()

function(_build_mongodb_driver buildMode)
    # 工作目录
    _cr_build_path(_CR_MONGODB_DRIVER_BUILD_DIR "mongodb-driver" ${buildMode})
    if(NOT EXISTS "${_CR_MONGODB_DRIVER_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_MONGODB_DRIVER_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MONGODB_DRIVER_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_MONGODB_DRIVER_INSTALL_DIR "mongodb-driver" ${buildMode})
    _cr_install_path(_CR_MONGODB_BSON_INSTALL_DIR "mongodb-bson" ${buildMode})
    if(NOT EXISTS "${_CR_MONGODB_DRIVER_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_MONGODB_DRIVER_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MONGODB_DRIVER_INSTALL_DIR}")
    endif()
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" ${_CR_ENABLE_SSL} ${_CR_ENABLE_SASL} -DBSON_ROOT_DIR=${_CR_MONGODB_BSON_INSTALL_DIR} -DCMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS=TRUE -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_MONGODB_DRIVER_INSTALL_DIR} -B "${_CR_MONGODB_DRIVER_BUILD_DIR}" "${_CR_MONGODB_DRIVER_SRC}"
            WORKING_DIRECTORY "${_CR_MONGODB_DRIVER_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(
        COMMAND ${_CR_MAKE} WORKING_DIRECTORY "${_CR_MONGODB_DRIVER_BUILD_DIR}"
    )
	execute_process(
        COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_MONGODB_DRIVER_BUILD_DIR}"
    )
endfunction()

function(_build_mongodb_driver_cxx buildMode)
   # 工作目录
    _cr_build_path(_CR_MONGODB_DRIVER_CXX_BUILD_DIR "mongodb-driver-cxx" ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_MONGODB_DRIVER_CXX_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_MONGODB_DRIVER_CXX_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MONGODB_DRIVER_CXX_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_MONGODB_DRIVER_CXX_INSTALL_DIR "mongodb-driver-cxx" ${buildMode})
    _cr_install_path(_CR_MONGODB_DRIVER_INSTALL_DIR "mongodb-driver" ${buildMode})
    _cr_install_path(_CR_MONGODB_BSON_INSTALL_DIR "mongodb-bson" ${buildMode})
    if(NOT EXISTS "${_CR_MONGODB_DRIVER_CXX_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_MONGODB_DRIVER_CXX_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_MONGODB_DRIVER_CXX_INSTALL_DIR}")
    endif()
    _cr_install_path(ENV{BOOST_ROOT} "boost" ${buildMode})
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" ${_CR_ENABLE_SSL} ${_CR_ENABLE_SASL} -DLIBMONGOC_DIR=${_CR_MONGODB_DRIVER_INSTALL_DIR} -DLIBBSON_DIR=${_CR_MONGODB_BSON_INSTALL_DIR} -BUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS=TRUE -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_MONGODB_DRIVER_CXX_INSTALL_DIR} -B "${_CR_MONGODB_DRIVER_CXX_BUILD_DIR}" "${_CR_MONGODB_DRIVER_CXX_SRC}"
            WORKING_DIRECTORY "${_CR_MONGODB_DRIVER_CXX_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(
        COMMAND ${_CR_MAKE} WORKING_DIRECTORY "${_CR_MONGODB_DRIVER_CXX_BUILD_DIR}"
    )
	execute_process(
        COMMAND ${_CR_MAKE} install WORKING_DIRECTORY "${_CR_MONGODB_DRIVER_CXX_BUILD_DIR}"
    )
endfunction()

_build_mongodb_bson("Release")
_build_mongodb_bson("Debug")

_build_mongodb_driver("Release")
_build_mongodb_driver("Debug")

_build_mongodb_driver_cxx("Release")
_build_mongodb_driver_cxx("Debug")
