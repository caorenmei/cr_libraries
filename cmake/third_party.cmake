if (NOT _CR_MSVC AND WIN32)
    set(_CR_MSVC 1)
endif()

foreach(libName boost cpp-netlib eluna lua mongodb mysql protobuf rapidxml)
    execute_process(COMMAND cmake -D_CR_MSVC=${_CR_MSVC} -P "${CMAKE_CURRENT_SOURCE_DIR}/${libName}.cmake" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()