cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(RING_LIST)

set(CMAKE_CONFIGURATION_TYPES Release)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1y")
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(test)