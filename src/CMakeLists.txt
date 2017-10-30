include_directories(${PROJECT_SOURCE_DIR}/src/common)
include_directories(${PROJECT_BINARY_DIR}/src/common)

include_directories(${PROJECT_BINARY_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_subdirectory(common)
add_subdirectory(example)
add_subdirectory(test)