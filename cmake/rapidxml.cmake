
set(_CR_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(_CR_RAPIDXML_SRC "${_CR_PROJECT_ROOT}/third_party/rapidxml")

include(common.cmake)

foreach(buildMode "Release" "Debug")
	_cr_install_path(_CR_RAPIDXML_INSTALL_DIR "rapidxml" ${buildMode})
	file(COPY 
        "${_CR_RAPIDXML_SRC}/rapidxml.hpp" 
        "${_CR_RAPIDXML_SRC}/rapidxml_iterators.hpp" 
        "${_CR_RAPIDXML_SRC}/rapidxml_print.hpp" 
        "${_CR_RAPIDXML_SRC}/rapidxml_utils.hpp"
        DESTINATION  
        "${_CR_RAPIDXML_INSTALL_DIR}/include/rapidxml"
    )
endforeach()