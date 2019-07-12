cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_STANDARD 14)

#For gmock
set(GMOCK_ROOT "/usr/src/gmock")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")

# Locate GTest
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${GMOCK_INCLUDE_DIRS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# include local
include_directories("/usr/local/include")

# libraries
SET(EXTERNAL_LIBS
        pthread
        glog
        gflags)

# Link runTests with what we want to test and the GTest and pthread library
add_definitions(-std=c++14 -ggdb -DUNIT_TEST)
include_directories(.)

add_executable(main main.cpp)
target_link_libraries(main ${EXTERNAL_LIBS})

add_executable(red_black_tree_test red_black_tree_test.cpp)
target_link_libraries(red_black_tree_test ${GTEST_LIBRARIES} ${EXTERNAL_LIBS})
