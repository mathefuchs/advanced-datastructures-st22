project(AdvancedDatastructuresST22 VERSION 0.1)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wmaybe-uninitialized")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-qual")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wredundant-decls")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnoexcept")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPARANOID ")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")
endif()

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")

# Main executable
include_directories(src)
add_executable(ads_programm_a src/main.cpp)
