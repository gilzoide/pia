cmake_minimum_required(VERSION 2.6)

project(pia)

file(GLOB src "*.c")
add_executable(pia ${src})
target_link_libraries(pia pega-texto gccjit)
