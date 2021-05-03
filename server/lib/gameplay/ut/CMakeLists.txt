cmake_minimum_required(VERSION 3.8)

find_package(GTest CONFIG)

add_executable(gameplay-ut ut.cpp cards_ut.cpp)

target_link_libraries(gameplay-ut GTest::gtest gameplay)

target_include_directories(gameplay-ut PRIVATE ${GTEST_INCLUDE_DIR} ${ROOT_SOURCE_DIR})

enable_testing()

add_test(gameplay-test gameplay-ut)
