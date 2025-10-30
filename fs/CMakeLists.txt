cmake_minimum_required(VERSION 3.10)
project(MyFS_FUSE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件（假设没有平台特定源文件）
set(SOURCES
    core.cpp
    main.cpp
)

#----------------------------------------
# 平台检测：Linux / Windows (WinFSP)
#----------------------------------------
if (WIN32)
    message(STATUS "Configuring for Windows with WinFSP...")
    
    # 1) 指定 WinFSP 头文件路径（默认安装目录）
    #    如果 WinFSP 安装到自定义目录，可改这里
    set(WINFSP_INCLUDE_DIR "C:/Program Files (x86)/WinFsp/inc/fuse3")
    set(WINFSP_LIB_DIR "C:/Program Files (x86)/WinFsp/lib/")

    include_directories(${WINFSP_INCLUDE_DIR})
    link_directories(${WINFSP_LIB_DIR})
    
    # 2) 新建可执行文件
    add_executable(myfs ${SOURCES})

    # 3) 链接 winfsp_fuse 以及正常的 ws2_32 等库
    #    （实际依赖可能因版本而异，可在 WinFSP 安装目录中查看）
    target_link_libraries(myfs PRIVATE winfsp-x64)

else() # Unix / Linux / macOS
    message(STATUS "Configuring for Linux/macOS with FUSE3...")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FUSE3 REQUIRED fuse3)

    include_directories(${FUSE3_INCLUDE_DIRS})
    add_definitions(${FUSE3_CFLAGS_OTHER})

    add_executable(myfs ${SOURCES})
    target_link_libraries(myfs PRIVATE ${FUSE3_LIBRARIES})
endif()

