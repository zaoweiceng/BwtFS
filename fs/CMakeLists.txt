# 跨平台FUSE文件系统构建配置
# 支持 Windows (WinFSP)、macOS (macFUSE) 和 Linux (libfuse3)

cmake_minimum_required(VERSION 3.10)
project(MyFS_FUSE LANGUAGES CXX)

# 设置C++标准为C++20，支持现代C++特性
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件列表（所有平台共享）
set(SOURCES
    core.cpp
    main.cpp
)

#----------------------------------------
# 跨平台配置：Windows / macOS / Linux
#----------------------------------------

if (WIN32)
    # Windows平台配置 (WinFSP)
    message(STATUS "Configuring for Windows with WinFSP...")

    # 指定WinFSP头文件路径（默认安装目录）
    # 如果WinFSP安装到自定义目录，需要修改这些路径
    set(WINFSP_INCLUDE_DIR "C:/Program Files (x86)/WinFsp/inc/fuse3")
    set(WINFSP_LIB_DIR "C:/Program Files (x86)/WinFsp/lib/")

    # 添加头文件和库文件搜索路径
    include_directories(${WINFSP_INCLUDE_DIR})
    link_directories(${WINFSP_LIB_DIR})

    # 创建可执行文件
    add_executable(myfs ${SOURCES})

    # 链接WinFSP库以及Windows网络库
    target_link_libraries(myfs PRIVATE
        winfsp-x64    # WinFSP核心库
        ws2_32        # Windows Sockets库
        )

    # 添加Windows特定的编译定义
    target_compile_definitions(myfs PRIVATE _WIN32)

elseif (APPLE)
    # macOS平台配置 (macFUSE)
    message(STATUS "Configuring for macOS with macFUSE...")

    # 使用pkg-config查找macFUSE库
    # 注意：macFUSE需要单独安装，通常通过brew install macfuse
    find_package(PkgConfig REQUIRED)

    # 尝试查找fuse库（macFUSE）
    pkg_check_modules(FUSE REQUIRED fuse)

    if (FUSE_FOUND)
        message(STATUS "Found macFUSE: ${FUSE_LIBRARIES}")
    else()
        message(FATAL_ERROR "macFUSE not found. Please install macFUSE: brew install macfuse")
    endif()

    # 添加头文件搜索路径
    include_directories(${FUSE_INCLUDE_DIRS})

    # 添加编译标志，包括FUSE必需的宏定义
    add_definitions(${FUSE_CFLAGS_OTHER} -D_FILE_OFFSET_BITS=64)

    # 创建可执行文件
    add_executable(myfs ${SOURCES})

    # 链接macFUSE库
    target_link_libraries(myfs PRIVATE ${FUSE_LIBRARIES})

    # 添加macOS特定的编译定义
    target_compile_definitions(myfs PRIVATE __APPLE__)

else()
    # Linux平台配置 (libfuse3)
    message(STATUS "Configuring for Linux with libfuse3...")

    # 使用pkg-config查找FUSE3库
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FUSE3 REQUIRED fuse3)

    if (FUSE3_FOUND)
        message(STATUS "Found FUSE3: ${FUSE3_LIBRARIES}")
    else()
        message(FATAL_ERROR "FUSE3 not found. Please install FUSE3 development package")
    endif()

    # 添加头文件搜索路径
    include_directories(${FUSE3_INCLUDE_DIRS})

    # 添加编译标志
    add_definitions(${FUSE3_CFLAGS_OTHER})

    # 创建可执行文件
    add_executable(myfs ${SOURCES})

    # 链接FUSE3库
    target_link_libraries(myfs PRIVATE ${FUSE3_LIBRARIES})

    # 添加Linux特定的编译定义
    target_compile_definitions(myfs PRIVATE __linux__)
endif()

#----------------------------------------
# 通用配置
#----------------------------------------

# 设置编译选项
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # GCC/Clang通用警告和优化选项
    target_compile_options(myfs PRIVATE
        -Wall                # 启用所有警告
        -Wextra             # 启用额外的警告
        -Wno-unused-parameter  # 忽略未使用参数警告（避免FUSE回调函数的参数警告）
        -O2                 # 优化级别2
        )

    # 调试模式下添加调试信息
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(myfs PRIVATE -g)
    endif()
endif()

# 设置输出目录
set_target_properties(myfs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}  # 可执行文件输出目录
)

# 安装规则（可选）
install(TARGETS myfs
    RUNTIME DESTINATION bin
    COMPONENT applications
)

#----------------------------------------
# 构建信息
#----------------------------------------
message(STATUS "")
message(STATUS "Build Configuration Summary:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
if (WIN32)
    message(STATUS "  FUSE Implementation: WinFSP")
    message(STATUS "  WinFSP Include Dir: ${WINFSP_INCLUDE_DIR}")
    message(STATUS "  WinFSP Library Dir: ${WINFSP_LIB_DIR}")
elseif (APPLE)
    message(STATUS "  FUSE Implementation: macFUSE")
    message(STATUS "  macFUSE Libraries: ${FUSE_LIBRARIES}")
else()
    message(STATUS "  FUSE Implementation: libfuse3")
    message(STATUS "  FUSE3 Libraries: ${FUSE3_LIBRARIES}")
endif()
message(STATUS "")

