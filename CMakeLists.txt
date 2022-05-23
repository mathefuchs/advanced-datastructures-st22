cmake_minimum_required(VERSION 3.15.0)
project(AdvancedDatastructuresST22 VERSION 0.1.0)

# Compiler flags
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG -mtune=native -march=native")

# External projects
add_subdirectory(external/googletest)

# Main App
add_subdirectory(advanced-datastructures-st22)
add_subdirectory(advanced-datastructures-st22/tests)
