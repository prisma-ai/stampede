cmake_minimum_required(VERSION 3.15)
project(graph_proc)

set(CMAKE_CXX_STANDARD 17)

add_executable(graph_proc main.cpp src/Graph.h src/Test.h src/Trait.h src/Traversal.h src/Executor.h src/BFSLastRecentlyUsedGCPlan.h src/FuncHelper.h src/GCPlan.h src/Compose.h src/async/Pool.h src/async/Pool.cpp)

target_link_libraries(graph_proc pthread)