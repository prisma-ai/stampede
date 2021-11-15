cmake_minimum_required(VERSION 3.15)
project(graph_proc)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(static_tests src/static_tests.cpp src/Example.h)

target_link_libraries(static_tests pthread)

find_package( OpenCV REQUIRED )

add_executable(cv_samples src/cv_samples/cv_samples.cpp src/cv_samples/Ops.h src/cv_samples/Vanilla.h src/cv_samples/Optimized.h src/cv_samples/Stampede.h include/spd/All.h)
set_property(TARGET cv_samples PROPERTY CXX_STANDARD 17)
target_include_directories(cv_samples PRIVATE ${OpenCV_INCLUDE_DIRS} ${ORT_INCLUDE_DIRS} include)

target_link_libraries (cv_samples ${OpenCV_LIBS} ${ORT_LIBS} pthread)