cmake_minimum_required(VERSION 3.8)

project(TICHU_SERVER)

add_executable(tichu-server tichu_server.cpp)

target_link_libraries(tichu-server api)

target_include_directories(tichu-server PUBLIC ${ROOT_SOURCE_DIR} ${API_BINARY_DIR})

set_property(TARGET tichu-server PROPERTY CXX_STANDARD 20)
