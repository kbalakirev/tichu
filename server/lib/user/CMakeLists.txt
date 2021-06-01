cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project(USER)

add_library(user user.cpp)

target_link_libraries(user actors)
target_include_directories(user PUBLIC ${ROOT_SOURCE_DIR})
