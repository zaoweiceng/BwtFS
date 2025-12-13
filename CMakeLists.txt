cmake_minimum_required(VERSION 3.10)

# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Release") # 设置编译类型为Release
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
project(BWTFileSystemProject VERSION 1.0.0 LANGUAGES CXX)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/include")
AUX_SOURCE_DIRECTORY("${CMAKE_CURRENT_SOURCE_DIR}/src" DIR_SRCS)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # 将项目根目录映射为"."
    add_compile_options("-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=")
endif()

if(WIN32)
    # Windows下使用WS2_32库
    find_library(WS2_32_LIB 
        NAMES ws2_32 ws2_32.lib   # Windows 库名
        PATHS "C:/MinGW/x86_64-w64-mingw32/lib"  # MinGW 库路径
        REQUIRED
    )
endif()

# net: 对象存储服务
add_subdirectory(net)

# src： 核心代码
add_subdirectory(src)

# cmd： 命令行工具
add_subdirectory(cmd)

add_subdirectory(fs)

# test： 测试代码
# add_subdirectory(lib/googletest)
# add_subdirectory(test)

