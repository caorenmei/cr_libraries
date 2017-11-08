set(_CR_COMMON_PEOJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

function(_cr_install_path libPath libName buildMode)
    set(${libPath} "${_CR_COMMON_PEOJECT_DIR}/install/${buildMode}/${libName}" PARENT_SCOPE)
endfunction()

function(_cr_build_path libPath libName buildMode)
    set(${libPath} "${_CR_COMMON_PEOJECT_DIR}/build/${buildMode}/${libName}" PARENT_SCOPE)
endfunction()