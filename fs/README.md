# BwtFS - 跨平台FUSE文件系统

## 概述

BwtFS 是一个基于FUSE的跨平台文件系统项目，支持三大主流操作系统：
- **Windows**: WinFSP (Windows File System Proxy)
- **macOS**: macFUSE 2.9+
- **Linux**: libfuse3

本项目专注于保护用户隐私，实现反跟踪存储机制和不可恢复的数据访问系统，特别适用于多用户和高机密环境。

## 系统要求

### macOS 开发环境
- macOS 10.15 (Catalina) 或更高版本
- Xcode Command Line Tools
- Homebrew 包管理器
- CMake 3.10 或更高版本
- C++17 兼容的编译器 (Clang)

### Windows 开发环境
- Windows 10 或更高版本
- Visual Studio 2022
- WinFSP 运行时和开发包
- CMake 3.10 或更高版本

### Linux 开发环境
- Ubuntu 18.04+ / CentOS 7+ 或其他发行版
- libfuse3 开发包
- CMake 3.10 或更高版本
- GCC 7+ 或 Clang 5+

## 依赖安装

### macOS
```bash
# 安装 Homebrew (如果未安装)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装开发工具
brew install cmake pkg-config

# 安装 macFUSE (需要系统权限)
brew install --cask macfuse

```

### Windows
```bash
# 下载安装 WinFSP
https://github.com/winfsp/winfsp/releases
```

### Linux
```bash
sudo apt-get update
sudo apt-get install cmake pkg-config libfuse3-dev
```

## 编译说明

### 通用编译步骤
```bash
# 创建构建目录
mkdir build
cd build

# 配置项目 (自动检测平台和FUSE实现)
cmake ..

# 编译
make

# 编译完成后，可执行文件位于当前目录
ls -la myfs
```

### 平台特定输出

**macOS编译输出:**
```
-- Configuring for macOS with macFUSE...
-- Found macFUSE: fuse
--
-- Build Configuration Summary:
--   Platform: Darwin
--   Compiler: /usr/bin/c++
--   C++ Standard: 20
--   Build Type: Release
--   Install Prefix: /usr/local
--
--   FUSE Implementation: macFUSE
--   macFUSE Libraries: fuse
--
-- Configuring done (0.0s)
-- Generating done (0.0s)
[ 33%] Building CXX object CMakeFiles/myfs.dir/core.cpp.o
[ 66%] Building CXX object CMakeFiles/myfs.dir/main.cpp.o
[100%] Linking CXX executable myfs
```

## 使用方法

### 快速开始

#### Linux

```bash
# 1. 编译项目
mkdir build && cd build
cmake ..
make

# 2. 创建挂载点
mkdir ~/myfs_mount

# 3. 挂载文件系统
./myfs ~/myfs_mount

# 4. 在新终端中测试
cd ~/myfs_mount
echo "Hello BwtFS" > test.txt
cat test.txt
ls -la

# 5. 卸载文件系统
umount ~/myfs_mount
```

#### Windows
```bash
# 1. 编译项目
mkdir build && cd build
cmake ..
# 使用VS打开sln，然后生成解决方案

# 2. 拷贝必要的dll文件
# Winfsp安装目录： C:\Program Files (x86)\WinFsp\bin
# 根据系统选择文件：winfsp-x64.dll

# 3. 挂载文件系统
# windows环境下作为磁盘进行挂载
./myfs X:

# 4. 卸载文件系统
# 退出程序即可卸载
```




## 故障排除

### 常见问题

1. **macOS/macFUSE安装失败**
   ```bash
   # 解决方案: 手动下载安装
   # https://github.com/macfuse/macfuse/releases
   brew uninstall --cask macfuse
   brew install --cask macfuse
   ```

2. **编译错误: 找不到FUSE库**
   ```bash
   # macOS解决方案:
   pkg-config --modversion fuse

   # Linux解决方案:
   pkg-config --modversion fuse3
   ```

3. **挂载失败: 权限被拒绝**
   ```bash
   # macOS解决方案: 检查系统设置
   # System Settings → Privacy & Security → Allow kernel extensions

   # Linux解决方案: 检查fuse权限
   groups $USER  # 确保在fuse组中
   sudo usermod -aG fuse $USER
   ```

4. **Windows: WinFSP服务未启动**
   ```bash
   # 解决方案: 检查WinFSP服务
   services.msc  # 检查WinFsp服务状态
   ```

## 项目结构

```
BwtFS/fs/
├── CMakeLists.txt          # 跨平台构建配置
├── main.cpp                # FUSE操作实现和适配层
├── core.h                  # 文件系统核心接口
├── core.cpp                # 文件系统核心实现
├── demo.c                  # macOS FUSE参考实现
├── README.md               # 本文档
└── build/                  # 编译输出目录
    ├── CMakeCache.txt
    ├── Makefile
    └── myfs               # 可执行文件
```

## 开发指南

### 添加新功能
1. **在core.h/core.cpp中实现核心逻辑**
2. **在main.cpp中添加FUSE回调**
3. **为不同平台提供适配函数**
4. **更新CMakeLists.txt如果需要新依赖**
5. **添加测试用例和文档**

### 内存管理
- 使用智能指针避免内存泄漏
- RAII 模式管理资源
- 考虑使用内存池优化性能

### 线程安全
- 当前实现为单线程，生产环境需要添加互斥锁
- 文件描述符管理需要原子操作
- 考虑使用读写锁提升并发性能

### 错误处理
- 所有FUSE回调函数都应返回适当的错误码
- 使用负数表示错误，如 `-ENOENT`, `-EIO`
- 提供详细的错误日志记录

### 性能优化建议
- 添加文件缓存机制
- 优化大文件读写性能
- 实现异步I/O
- 考虑添加预取和延迟写入

## 构建系统详细说明

### CMake配置特性
- **平台自动检测**: 根据 `WIN32`、`APPLE`、`UNIX` 宏选择平台
- **FUSE版本检测**: 自动选择对应的FUSE库和版本
- **编译器支持**: 支持GCC、Clang、MSVC
- **依赖管理**: 使用pkg-config和FindPkgConfig模块

### 编译选项
- **Release模式**: 默认开启优化 (`-O2`)
- **Debug模式**: 可启用调试信息和断言
- **C++标准**: C++17 (主要项目) / C++20 (FUSE组件)
- **警告级别**: 启用常用警告和静态分析