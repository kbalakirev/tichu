cmake_minimum_required(VERSION 3.8)

include(${ROOT_SOURCE_DIR}/cmake/common.cmake)

set(CMAKE_CXX_STANDARD 20)

project(GRPC)

set(CMAKE_MODULE_PATH ${ROOT_SOURCE_DIR}/cmake)

add_library(grpc server.cpp)

target_link_libraries(grpc ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF} ${_REFLECTION})
target_include_directories(grpc PUBLIC ${ROOT_SOURCE_DIR})

set_property(TARGET tichu-server PROPERTY CXX_STANDARD 20)
