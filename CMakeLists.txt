cmake_minimum_required(VERSION 3.15)
project(graph_proc)

set(CMAKE_CXX_STANDARD 17)

add_executable(graph_proc main.cpp src/Graph.cpp src/Graph.h src/Test.h src/Trait.h)