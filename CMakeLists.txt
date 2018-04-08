cmake_minimum_required(VERSION 2.6)

project(pia)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wno-parentheses")

file(GLOB src "*.c")
add_executable(pia ${src})
target_link_libraries(pia pega-texto2 jit Judy)

