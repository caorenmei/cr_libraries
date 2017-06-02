
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_ELUNA_SRC "${_CR_PROJECT_ROOT}/third_party/eluna")

include(common.cmake)

foreach(buildMode "Release" "Debug")
	_cr_install_path(_CR_ELUNA_INSTALL_DIR "eluna" ${buildMode})
	file(COPY 
        "${_CR_ELUNA_SRC}/eluna" 
        DESTINATION  
        "${_CR_ELUNA_INSTALL_DIR}/include/"
    )
endforeach()

