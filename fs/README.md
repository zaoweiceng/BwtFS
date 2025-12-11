# BwtFS 文件系统 - FUSE 混合存储架构

## 🎯 项目概述

BwtFS FUSE 文件系统采用创新的**混合存储架构**，结合了内存文件系统（MemoryFS）和BwtFS的加密存储技术，实现了智能的文件分类存储策略：

- **系统临时文件** → 存储在 `memory_fs`（内存中，快速访问）
- **用户文件** → 存储在 `BwtFS`（加密存储，隐私保护）

### 核心特性

- **🔒 隐私保护**: 用户文件采用BwtFS加密存储，无法被追踪或恢复
- **⚡ 高性能**: 系统临时文件直接存储在内存中，避免加密开销
- **🔄 智能分类**: 自动识别文件类型并选择最优存储策略
- **🌐 跨平台**: 支持Windows (WinFSP)、macOS (macFUSE)、Linux (libfuse3)
- **💾 COW策略**: 采用Copy-on-Write机制保护数据完整性
- **🛡️ 数据安全**: 多层验证机制确保数据完整性

### 项目架构

```
BwtFS Mounter (FUSE接口层)
├── MemoryFS Manager (内存文件管理)
├── BwtFS Manager (BwtFS文件管理)
└── File Manager (统一文件接口)
    ├── 智能分类器
    ├── 引用计数管理
    └── 文件状态追踪
```

## 🚀 快速开始

### 编译项目

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make

# 可执行文件位于 build/bin/ 目录下
ls -la build/bin/
# bwtfs_mount.exe  bwtfs_cmd.exe  libBWTFileSystemProject.a  BWTFileSystemProject_run.exe
```

### 挂载文件系统

```bash
# 1. 创建BwtFS系统文件（如果不存在）
./build/bin/bwtfs_cmd create demo.bwt 256  # 创建256MB的BwtFS系统

# 2. 挂载文件系统到X盘（Windows）
./build/bin/bwtfs_mount.exe X: ./demo.bwt ./fs.json

# 3. 使用文件系统
# 现在可以在X:盘中正常使用文件系统
# 小文件（<100KB）- 快速存储
# 大文件（>100KB）- 自动COW处理和加密存储
```

### 卸载文件系统

```bash
# 直接退出程序即可自动卸载
# 或使用 Ctrl+C 中断进程
# Linux环境下使用umount
```

## 🔧 系统要求

### 开发环境
- **编译器**: C++20 或更高版本
- **构建工具**: CMake 3.10 或更高版本
- **平台**: Windows 10+, macOS 10.15+, 或 Linux

### 运行时依赖
- **Windows**: WinFSP 运行时
- **macOS**: macFUSE 2.9+
- **Linux**: libfuse3

## 🏗️ 架构设计

### 混合存储策略

| 文件类型 | 存储后端 | 大小阈值 | 特性 |
|---------|---------|----------|------|
| 系统临时文件 | MemoryFS | < 100KB | 快速访问，不占用加密空间 |
| 用户文件 | BwtFS | > 100KB | 加密存储，隐私保护 |

### 核心组件

#### 1. MemoryFS (内存文件系统)
- **用途**: 处理小文件和临时文件
- **优势**: 极快的读写速度
- **限制**: 内存容量有限

#### 2. BwtFS Manager (BwtFS文件管理器)
- **用途**: 处理大文件和用户数据
- **优势**: 加密存储，隐私保护
- **特性**: 分布式存储，抗追踪

#### 3. File Manager (统一文件接口)
- **功能**: 文件分类、状态管理、统一接口
- **特性**: 智能分类算法、COW支持

### COW (Copy-on-Write) 机制

```cpp
// 读取原文件内容到临时文件
auto data = read_tree.read(index, chunk_size);
memory_fs_.write(temp_fd, data.data(), data.size());

// 写入新的内容
memory_fs_.write(temp_fd, buf, size);

// 关闭时：写入BwtFS，更新token
tree.write(temp_data.data(), final_size);
new_token = tree.get_token();
```

## 🔍 故障排除

### 常见问题

1. **文件写入失败**
   - 检查BwtFS系统文件是否正常
   - 验证磁盘空间是否充足
   - 查看日志中的错误信息

2. **文件读取错误**
   - 检查文件token是否有效
   - 验证文件是否存在于正确的存储后端
   - 确认文件权限设置正确

3. **性能问题**
   - 大文件写入时检查内存使用情况
   - 监控COW处理的效率
   - 调整文件大小阈值

### 调试日志

日志文件位置: `bwtfs-YYYYMMDD.log` 与`bwtfs_mount`程序同级

```bash
# 查看实时日志
tail -f build/bin/bwtfs-20251210.log

# 搜索特定错误
grep -i "error\|failed\|corrupt" build/bin/bwtfs-20251210.log
```

## 📚 API 参考

详细的API文档请参考: [README_DEV.md](README_DEV.md)
