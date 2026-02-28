cmake_minimum_required(VERSION 3.10)
project(SimpleDBMS VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

# Find all source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# Output executable
add_executable(dbms ${SRC_FILES})
