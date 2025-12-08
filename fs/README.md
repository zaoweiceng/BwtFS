# BwtFS FUSE 文件系统 - 混合存储架构

## 🎯 概述

BwtFS FUSE 文件系统采用创新的**混合存储架构**，智能地将文件分类存储，以优化性能和存储效率：

- **系统临时文件** → 存储在 `memory_fs`（内存中）
- **用户文件** → 存储在 `BwtFS`（加密存储）

本项目支持三大主流操作系统：
- **Windows**: WinFSP (Windows File System Proxy)
- **macOS**: macFUSE 2.9+
- **Linux**: libfuse3

专注于保护用户隐私，实现反跟踪存储机制和不可恢复的数据访问系统。

## 系统要求

### macOS 开发环境
- macOS 10.15 (Catalina) 或更高版本
- Xcode Command Line Tools
- Homebrew 包管理器
- CMake 3.10 或更高版本
- C++20 兼容的编译器

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

# 编译完成后，可执行文件位于当前目录的bin目录下
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

## 🏗️ 混合存储架构

### 架构设计理念

混合存储架构解决了跨平台文件系统中的关键问题：

1. **系统临时文件污染**：Finder、资源管理器等工具创建大量临时文件
2. **存储空间浪费**：系统文件占用宝贵的加密存储空间
3. **性能问题**：系统文件的异步处理导致文件管理器兼容性问题

### 文件分类策略

| 文件类型 | 存储后端 | 特点 | 示例 |
|---------|---------|------|------|
| 系统临时文件 | `memory_fs` | 快速访问，占用内存，不占用加密存储空间 | `._.DS_Store`, `Thumbs.db`, `desktop.ini` |
| 用户文件 | `BwtFS` | 加密存储，隐私保护，抗溯源 | 用户文档、图片、视频等 |

### 跨平台系统文件支持

#### macOS 系统文件
- `._.*` - 资源分支文件（Resource Fork）
- `.DS_Store` - 桌面服务存储文件
- `.TemporaryItems/*` - 临时项目文件夹
- `.vscode/*` - VS Code 临时文件

#### Windows 系统文件
- `Thumbs.db` - 缩略图缓存文件
- `desktop.ini` - 桌面配置文件
- `~$*` - Office 临时文件

#### 通用开发工具
- `.vscode/*` - VS Code 临时文件
- `~$*` - Office 临时文件
- `.TemporaryItems/*` - 临时项目文件夹

### 技术实现

#### 智能文件分类
```cpp
bool isSystemTempFile(const std::string& path) {
    std::string basename = path.substr(path.find_last_of('/') + 1);

    // macOS系统文件
    if (basename.find("._") == 0 || basename == ".DS_Store" ||
        basename.find(".TemporaryItems") == 0) {
        return true;
    }

    // Windows系统文件
    if (basename == "Thumbs.db" || basename == "desktop.ini" ||
        basename.find("~$") == 0) {
        return true;
    }

    return false;
}
```

#### 混合读取策略
```cpp
FileNode getFileNode(const std::string& path) {
    if (isSystemTempFile(path)) {
        // 从 memory_fs 获取
        return getFromMemoryFS(path);
    } else {
        // 从 BwtFS 获取
        return getFromBwtFS(path);
    }
}
```

#### 统一文件系统接口
- `create()` - 智能选择存储后端
- `read()` - 从对应存储读取数据
- `write()` - 写入到对应存储
- `list_files_in_dir()` - 合并两个存储的文件列表
- `remove()` - 从对应存储删除文件

### 架构优势

#### 1. **存储效率提升**
- 系统临时文件不占用宝贵的加密存储空间
- 内存访问速度更快，提升系统响应性
- 减少不必要的加密操作

#### 2. **Finder 兼容性**
- 系统文件立即可用，无异步延迟
- 避免了 Finder 显示问题
- 支持各平台的文件管理器

#### 3. **跨平台支持**
- 统一处理各操作系统的临时文件
- 自动识别和分类系统文件
- 无需手动配置

#### 4. **隐私保护**
- 用户文件仍然获得 BwtFS 的完整隐私保护
- 系统文件隔离存储，不影响用户数据安全
- 混合架构不影响隐私保护特性

#### 5. **透明性**
- 用户无感知的文件分类
- 统一的文件系统接口
- 保持原有的 POSIX 语义

## 🚀 使用方法

### 快速开始

#### 1. 使用 MemoryFS（测试模式）

```bash
# 1. 编译项目
mkdir build && cd build
cmake ..
make

# 2. 创建挂载点
mkdir /tmp/memory_fs

# 3. 挂载 MemoryFS（仅内存存储）
./bwtfs_mount /tmp/memory_fs

# 4. 在新终端中测试
cd /tmp/memory_fs
echo "Hello MemoryFS" > test.txt
mkdir test_dir
ls -la
cat test.txt

# 5. 卸载文件系统
umount /tmp/memory_fs  # Linux
# 或在 macOS 上: umount /tmp/memory_fs
```

#### 2. 使用 BwtFS 混合存储（推荐）

```bash
# 1. 创建 BwtFS 系统文件（在项目根目录）
cd ../../
mkdir -p build && cd build
cmake ..
make
./bwtfs_cmd

# 2. 创建 BwtFS 系统文件
# 根据提示输入：
# - 文件系统路径：./test.bwt
# - 文件系统大小：1024（MB）

# 3. 挂载混合存储文件系统
mkdir /tmp/bwtfs
./bwtfs_mount ../../test.bwt /tmp/bwtfs

# 4. 在新终端中测试混合存储
cd /tmp/bwtfs

# 用户文件（将加密存储到 BwtFS）
echo "用户重要数据" > user_document.txt
cp ~/photo.jpg .
mkdir user_files

# 系统临时文件（自动路由到内存存储）
# 这些文件会由 Finder/系统工具自动创建
# .DS_Store, Thumbs.db, ._*, 等

# 查看文件列表
ls -la
# 可以看到用户文件和系统临时文件都正常显示

# 5. 卸载文件系统
umount /tmp/bwtfs
```

#### 3. Linux 系统完整示例

```bash
# 1. 编译项目
mkdir build && cd build
cmake ..
make

# 2. 创建挂载点
mkdir ~/bwtfs_mount

# 3. 挂载混合存储文件系统
./bwtfs_mount ~/bwtfs_mount

# 4. 测试用户文件（BwtFS 加密存储）
cd ~/bwtfs_mount
echo "Hello BwtFS" > user_file.txt
mkdir user_directory
cp ~/Documents/important.pdf .

# 5. 观察系统文件自动路由到内存存储
# 使用文件管理器访问目录，会自动创建系统临时文件
# 这些文件不会占用 BwtFS 存储空间

# 6. 验证存储效果
ls -la
# 用户文件和系统文件都可见，但存储在不同的后端

# 7. 卸载文件系统
umount ~/bwtfs_mount
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
./bwtfs_mount X:

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
- **C++标准**:  C++20
- **警告级别**: 启用常用警告和静态分析
