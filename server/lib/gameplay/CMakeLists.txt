cmake_minimum_required(VERSION 3.8)

project(GAMEPLAY)

add_subdirectory(ut)

add_library(gameplay gameplay.cpp cards/cards.cpp)

target_include_directories(gameplay PRIVATE ${ROOT_SOURCE_DIR})
