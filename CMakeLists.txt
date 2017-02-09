cmake_minimum_required (VERSION 3.5.1)
project (leptjson  CXX)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -ansi -pedantic -g -Wall")
endif()

add_compile_options(-std=c++11)
add_compile_options(-g)
add_library(leptjson source/leptjson.cpp)
add_executable(leptjson_test test/test.cpp)
target_link_libraries(leptjson_test leptjson)
