cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project(TABLE)

add_library(table table.cpp)

target_link_libraries(table actors gameplay)
target_include_directories(table PUBLIC ${ROOT_SOURCE_DIR})
