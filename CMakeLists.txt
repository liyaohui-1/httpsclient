cmake_minimum_required(VERSION 3.10)

project(demo_https_client)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output)

include_directories(${CMAKE_SOURCE_DIR}/../install/include)
include_directories(${CMAKE_SOURCE_DIR}/../install_zlib/include)
include_directories(${CMAKE_SOURCE_DIR}/../)

link_directories(${CMAKE_SOURCE_DIR}/../install/lib)
link_directories(${CMAKE_SOURCE_DIR}/../install_zlib/lib)

set(SRC_FILES ./main.cpp ./https_client.cpp ./compress.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} curl)
target_link_libraries(${PROJECT_NAME} z)