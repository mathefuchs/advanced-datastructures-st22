cmake_minimum_required(VERSION 3.15.0)
project(AdvancedDatastructuresST22 VERSION 0.1.0)

# Enforce C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -mtune=native -march=native")

# External projects
add_subdirectory(external/googletest)

# Main App
add_subdirectory(advanced-datastructures-st22)
add_subdirectory(advanced-datastructures-st22/tests)

# Code coverage
option(CODE_COVERAGE "Enable coverage reporting" OFF)

# Print compiler flags
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
