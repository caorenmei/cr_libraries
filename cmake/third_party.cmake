
foreach(libName  boost cpp-netlib eluna lua mongodb mysql openssl protobuf rapidxml zookeeper)
    execute_process(COMMAND cmake -D_CR_MSVC=${_CR_MSVC} -P "${CMAKE_CURRENT_SOURCE_DIR}/${libName}.cmake" WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endforeach()