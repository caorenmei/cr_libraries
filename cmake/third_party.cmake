if (NOT _CR_MSVC AND WIN32)
	set(_CR_MSVC 1)
endif()
foreach(libName boost cpp-netlib lua mysql protobuf rapidxml rocksdb selene xlnt)
    execute_process(
        COMMAND cmake -D_CR_MSVC=${_CR_MSVC} -P "${CMAKE_CURRENT_SOURCE_DIR}/${libName}.cmake" 
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()
if(NOT(_CR_MSVC EQUAL 1))
    foreach(libName uuid gperftools)
        execute_process(COMMAND cmake -P "${CMAKE_CURRENT_SOURCE_DIR}/${libName}.cmake" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    endforeach()
endif()