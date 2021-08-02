cmake_minimum_required(VERSION 3.15)
project(graph_proc)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(graph_proc src/main.cpp src/Example.h src/Test.h)

target_link_libraries(graph_proc pthread)