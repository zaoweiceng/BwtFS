# BwtFS 命令行工具 CMake 配置
set(BINARY bwtfs)

# 收集所有源文件
file(GLOB_RECURSE CMD_SOURCES
    LIST_DIRECTORIES false
    "*.h"
    "*.cpp"
    "*.hpp"
)

# 明确列出源文件以确保构建顺序
set(SOURCES
    main.cpp
    command_handler.cpp
    file_operations.cpp
    ui_helper.cpp
)

# 创建可执行文件
add_executable(${BINARY} ${SOURCES})

# 添加测试
add_test(NAME ${BINARY} COMMAND ${BINARY})

# 链接BwtFS核心库
target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib)

# 设置编译选项
target_compile_options(${BINARY} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# 设置C++标准
set_property(TARGET ${BINARY} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${BINARY} PROPERTY CXX_STANDARD_REQUIRED ON)

# 添加包含目录
target_include_directories(${BINARY} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 设置输出目录
set_target_properties(${BINARY} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)