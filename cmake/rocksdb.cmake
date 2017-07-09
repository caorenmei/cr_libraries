
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_ROCKSDB_SRC "${_CR_PROJECT_ROOT}/third_party/rocksdb/rocksdb-5.5.1")

include(common.cmake)

 # Makefile
if(_CR_MSVC EQUAL 1)
    set(_CR_MAKEFILE "NMake Makefiles")
    set(_CR_MAKE "nmake")
    set(_CR_ROCKSDB_LIB "rocksdb.lib")
else()
    set(_CR_MAKEFILE "Unix Makefiles")
    set(_CR_MAKE "make")
    set(_CR_ROCKSDB_LIB "librocksdb.a")
endif()

# 编译Bson
function(_build_rocksdb buildMode)
    # 工作目录
    _cr_build_path(_CR_ROCKSDB_BUILD_DIR rocksdb ${buildMode})
    # 编译目录
    if(NOT EXISTS "${_CR_ROCKSDB_BUILD_DIR}")
        file(MAKE_DIRECTORY "${_CR_ROCKSDB_BUILD_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_ROCKSDB_BUILD_DIR}")
    endif()
    # 生成cmake
    _cr_install_path(_CR_ROCKSDB_INSTALL_DIR rocksdb ${buildMode})
    if(NOT EXISTS "${_CR_ROCKSDB_INSTALL_DIR}")
        file(MAKE_DIRECTORY "${_CR_ROCKSDB_INSTALL_DIR}")
        message(STATUS "MAKE_DIRECTORY ${_CR_ROCKSDB_INSTALL_DIR}")
    endif()
    execute_process(
        COMMAND cmake -G "${_CR_MAKEFILE}" -DCMAKE_BUILD_TYPE=${buildMode} -DCMAKE_INSTALL_PREFIX=${_CR_ROCKSDB_INSTALL_DIR} -DFAIL_ON_WARNINGS=OFF -B "${_CR_ROCKSDB_BUILD_DIR}" "${_CR_ROCKSDB_SRC}"
            WORKING_DIRECTORY "${_CR_ROCKSDB_BUILD_DIR}"
    )
    # 编译 & 安装
    execute_process(COMMAND ${_CR_MAKE} rocksdb WORKING_DIRECTORY "${_CR_ROCKSDB_BUILD_DIR}")
    file(COPY "${_CR_ROCKSDB_BUILD_DIR}/${_CR_ROCKSDB_LIB}" DESTINATION "${_CR_ROCKSDB_INSTALL_DIR}/lib/")
    file(COPY "${_CR_ROCKSDB_SRC}/include/" DESTINATION "${_CR_ROCKSDB_INSTALL_DIR}/include/")
endfunction()

_build_rocksdb("Release")
_build_rocksdb("Debug")