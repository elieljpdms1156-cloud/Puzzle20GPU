cmake_minimum_required(VERSION 3.18.1)
project("puzzle20_gpu")

# OpenCL
find_library(OPENCL_LIB OpenCL)

add_library(puzzle20_gpu SHARED gpu_search.cpp)
target_compile_options(puzzle20_gpu PRIVATE -O3)
target_link_libraries(puzzle20_gpu ${OPENCL_LIB} android log)
