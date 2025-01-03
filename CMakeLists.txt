cmake_minimum_required(VERSION 3.10)

project(demo_https_client)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output)

include_directories(${CMAKE_SOURCE_DIR}/../curl_install/include)
include_directories(${CMAKE_SOURCE_DIR}/../zlib_install/include)
include_directories(${CMAKE_SOURCE_DIR}/../)

link_directories(${CMAKE_SOURCE_DIR}/../curl_install/lib)
link_directories(${CMAKE_SOURCE_DIR}/../zlib_install/lib)

set(SRC_FILES ./main.cpp ./https_client.cpp ./compress.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} curl)
target_link_libraries(${PROJECT_NAME} minizip)
target_link_libraries(${PROJECT_NAME} z)
target_link_libraries(${PROJECT_NAME} pthread)