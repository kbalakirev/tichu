cmake_minimum_required(VERSION 3.8)

project(GAMEPLAY)

add_subdirectory(ut)

add_library(gameplay gameplay.cpp cards/cards.cpp cards/impl.cpp)

target_include_directories(gameplay PRIVATE ${ROOT_SOURCE_DIR})

set_property(TARGET gameplay PROPERTY CXX_STANDARD 20)
