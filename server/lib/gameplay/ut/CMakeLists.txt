cmake_minimum_required(VERSION 3.8)

find_package(GTest CONFIG)

if (GTest_FOUND)
    add_executable(gameplay-ut ut.cpp cards_ut.cpp state_ut.cpp)

    target_link_libraries(gameplay-ut GTest::gtest gameplay)

    target_include_directories(gameplay-ut PRIVATE ${GTEST_INCLUDE_DIR} ${ROOT_SOURCE_DIR})

    enable_testing()

    set_property(TARGET gameplay-ut PROPERTY CXX_STANDARD 20)

    add_test(gameplay-test gameplay-ut)

else()
    message(STATUS "gtest not found")

endif()
