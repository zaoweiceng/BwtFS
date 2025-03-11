cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(BWTFileSystemProject VERSION 1.0.0 LANGUAGES CXX)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
AUX_SOURCE_DIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/src" DIR_SRCS)

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(lib/googletest)
