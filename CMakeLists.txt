cmake_minimum_required (VERSION 2.6)

set(SOURCES blocksaver.cpp blocksavermain.cpp)
 
add_executable(blocksaver ${SOURCES})



include_directories(./)
include_directories(test)
file(GLOB TEST_SOURCES test/*.cpp)
set(TEST_SOURCES blocksaver.cpp test/gtest/gtest-all.cc ${TEST_SOURCES})

add_executable(blocksavertest ${TEST_SOURCES})
target_link_libraries(blocksavertest pthread )