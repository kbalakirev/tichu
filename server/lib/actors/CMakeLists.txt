cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project(ACTORS)

set(CMAKE_MODULE_PATH ${ROOT_SOURCE_DIR}/cmake)

find_package(CAF COMPONENTS core REQUIRED)

add_library(actors actor.cpp)

target_link_libraries(actors ${CAF_LIBRARIES})
target_include_directories(actors PUBLIC ${ROOT_SOURCE_DIR} ${CAF_INCLUDE_DIRS})
