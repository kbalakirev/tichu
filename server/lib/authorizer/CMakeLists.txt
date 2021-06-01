cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project(AUTHORIZER)

add_library(authorizer authorizer.cpp)

target_link_libraries(authorizer actors)
target_include_directories(authorizer PUBLIC ${ROOT_SOURCE_DIR})
