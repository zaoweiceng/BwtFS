# BwtFS FS 开发文档

## 📋 目录

- [架构概述](#架构概述)
- [核心类详解](#核心类详解)
- [API 接口文档](#api-接口文档)
- [COW 机制详解](#cow-机制详解)
- [文件描述符管理](#文件描述符管理)
- [错误处理策略](#错误处理策略)
- [内存管理](#内存管理)
- [并发控制](#并发控制)
- [性能优化](#性能优化)
- [调试指南](#调试指南)
- [配置参数](#配置参数)

## 🏗️ 架构概述

### 系统架构图

```
┌─────────────────────────────────────────────────────────────┐
│                    FUSE Interface Layer                     │
├─────────────────────────────────────────────────────────────┤
│                  BwtFSMounter Class                         │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────────────┐    ┌─────────────────────────────┐ │
│  │   MemoryFS Manager  │    │    BwtFS Manager            │ │
│  │                     │    │                             │ │
│  │ • 临时文件存储       │    │ • 用户文件加密存储            │ │
│  │ • 内存管理           │    │ • COW 机制                  │ │
│  │ • 快速访问           │    │ • Token 管理                │ │
│  └─────────────────────┘    └─────────────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                   FileManager                               │
│  • JSON 元数据管理      • 文件状态追踪      • 引用计数管理     │
└─────────────────────────────────────────────────────────────┘
```

### 核心组件交互

1. **文件路径解析** → BwtFSMounter::normalize_path()
2. **存储策略选择** → isSystemTempFile() 判断
3. **文件描述符分配** → 独立 FD 管理机制
4. **数据读写操作** → MemoryFS 或 BwtFS 处理
5. **元数据更新** → FileManager 状态同步

## 📚 核心类详解

### BwtFSMounter 类

主要的文件系统挂载器类，负责统一管理 MemoryFS 和 BwtFS。

#### 私有成员变量

```cpp
class BwtFSMounter {
private:
    SystemManager system_manager_;          // BwtFS 系统管理器
    FileManager file_manager_;              // 文件元数据管理器
    MemoryFS memory_fs_;                    // 内存文件系统实例

    // 文件描述符映射表
    std::unordered_map<int, std::string> fd_map_;           // FD -> 文件路径
    std::unordered_map<std::string, int> path_fd_map_;     // 文件路径 -> 主 FD
    std::unordered_map<std::string, int> path_ref_count_;  // 文件路径 -> 引用计数
    std::unordered_map<int, BwtFS::Node::bw_tree*> fd_tree_map_; // FD -> bw_tree 对象
    std::unordered_map<int, int> fd_to_memory_fd_map_;     // BwtFS FD -> Memory FD

    int next_fd_ = 1;                     // 下一个可用的文件描述符
};
```

#### 关键设计决策

1. **独立 FD 分配**: 每个 `open()` 操作都获得独立的 FD，避免 FD 重用导致的竞态条件
2. **路径到 FD 映射**: 维护主 FD 映射，支持多次打开同一文件
3. **引用计数**: 跟踪文件的打开次数，正确管理资源释放
4. **分层存储**: 根据文件特性自动选择存储后端

### MemoryFS 类

内存文件系统实现，处理小文件和临时文件。

#### 核心数据结构

```cpp
class MemoryFS {
public:
    struct File {
        std::string name;           // 文件名
        std::vector<char> data;     // 文件数据
        bool is_directory;          // 目录标识
    };

private:
    std::unordered_map<std::string, File> files_;    // 文件存储
    std::unordered_map<int, std::string> fd_map_;    // FD 映射
    int next_fd_ = 1;                               // FD 计数器
};
```

#### 特性

- **快速访问**: 所有数据存储在内存中，读写延迟极低
- **简单结构**: 使用 unordered_map 提供高效的查找性能
- **目录支持**: 通过 `is_directory` 标识支持目录结构

## 🔧 API 接口文档

### 文件操作接口

#### open()

```cpp
int open(const std::string& path, int flags = 0);
```

**功能**: 打开文件并返回文件描述符

**参数**:
- `path`: 文件路径（支持相对和绝对路径）
- `flags`: 打开标志（预留参数）

**返回值**:
- 成功: 返回有效的文件描述符（>= 1）
- 失败: 返回 -1

**实现细节**:
1. 路径标准化处理
2. 文件存在性检查
3. 存储策略选择（MemoryFS vs BwtFS）
4. FD 映射表更新
5. 引用计数管理

**示例**:
```cpp
int fd = mounter.open("/test.txt");
if (fd > 0) {
    // 文件打开成功
}
```

#### read()

```cpp
int read(int fd, char* buf, size_t size);
int read(int fd, char* buf, size_t size, off_t offset);
```

**功能**: 从文件读取数据

**参数**:
- `fd`: 文件描述符
- `buf`: 读取缓冲区
- `size`: 读取字节数
- `offset`: 可选，读取偏移量

**返回值**:
- 成功: 返回实际读取的字节数
- 失败: 返回 -1

**实现细节**:
1. FD 有效性验证
2. 存储后端识别（MemoryFS 或 BwtFS）
3. 大小和边界检查
4. 数据拷贝到缓冲区

#### write()

```cpp
int write(int fd, const char* buf, size_t size);
int write(int fd, const char* buf, size_t size, off_t offset);
```

**功能**: 向文件写入数据

**参数**:
- `fd`: 文件描述符
- `buf`: 写入数据缓冲区
- `size`: 写入字节数
- `offset`: 可选，写入偏移量

**返回值**:
- 成功: 返回实际写入的字节数
- 失败: 返回 -1

**实现细节**:
1. **COW 机制**: 对于 BwtFS 文件，自动启用 Copy-on-Write
2. **临时文件**: 创建内存临时文件存储写入数据
3. **数据验证**: 检查写入数据的完整性
4. **状态更新**: 更新文件状态和元数据

#### close()

```cpp
int close(int fd);
```

**功能**: 关闭文件并保存数据

**参数**:
- `fd`: 文件描述符

**返回值**:
- 成功: 返回 0
- 失败: 返回 -1

**实现细节**:
1. **重复关闭检测**: 防止多次关闭导致的资源释放问题
2. **COW 最终化**: 将临时文件数据写入 BwtFS
3. **Token 更新**: 获取并保存新的文件 token
4. **资源清理**: 清理所有相关的映射和计数器

**关键逻辑**:
```cpp
// 检测重复 finalize
if (current_token != "memory" && !current_token.empty()) {
    LOG_WARNING << "[close] DUPLICATE FINALIZE DETECTED!";
    return 0;
}

// COW 最终化处理
tree.write(temp_data.data(), final_size);
new_token = tree.get_token();
```

### 目录操作接口

#### mkdir()

```cpp
int mkdir(const std::string& path);
```

**功能**: 创建目录

**实现细节**:
1. 路径验证和标准化
2. 父目录存在性检查
3. 递归目录创建
4. 元数据更新

#### list_files_in_dir()

```cpp
std::vector<std::string> list_files_in_dir(const std::string& dir_path);
```

**功能**: 列出目录内容

**返回值**: 目录下的文件和子目录列表

### 工具函数

#### normalize_path()

```cpp
std::string normalize_path(const std::string& path);
```

**功能**: 标准化文件路径

**处理规则**:
1. 将反斜杠转换为正斜杠
2. 移除重复的斜杠
3. 处理相对路径中的 `.` 和 `..`
4. 确保路径以 `/` 开头

#### isSystemTempFile()

```cpp
bool isSystemTempFile(const std::string& path);
```

**功能**: 判断是否为系统临时文件

**当前策略**: 返回 false（所有文件都存储在 BwtFS 中）

**扩展性**: 可根据需要添加系统文件识别规则

## 🔄 COW 机制详解

Copy-on-Write 机制是 BwtFS 的核心特性，确保数据完整性和并发安全。

### COW 工作流程

```
1. 文件打开
   ├── 检查文件是否存在
   ├── 读取原文件内容到内存
   └── 创建临时文件用于写入

2. 数据写入
   ├── 所有写入操作指向临时文件
   ├── 原文件保持不变
   └── 支持增量写入和随机访问

3. 文件关闭
   ├── 验证临时文件完整性
   ├── 将临时文件写入 BwtFS
   ├── 更新文件 token
   └── 清理临时资源
```

### COW 关键代码实现

```cpp
// 1. 创建临时文件
int temp_fd = memory_fs_.create(temp_path);

// 2. 读取原文件内容（如果存在）
if (current_token != "memory" && !current_token.empty()) {
    auto tree = BwtFS::Node::bw_tree(current_token, &system_manager_);
    auto data = tree.read(index, chunk_size);
    memory_fs_.write(temp_fd, data.data(), data.size());
}

// 3. 写入新数据
memory_fs_.write(temp_fd, buf, size);

// 4. 关闭时最终化
tree.write(temp_data.data(), final_size);
new_token = tree.get_token();
```

### COW 优势

1. **数据安全**: 原文件在写入过程中始终保持完整
2. **并发支持**: 多个进程可以同时读取同一文件
3. **事务性**: 写入操作要么完全成功，要么完全失败
4. **版本控制**: 通过 token 机制支持文件版本管理

## 📊 文件描述符管理

### FD 分配策略

- **独立分配**: 每个 `open()` 调用获得独立的 FD
- **顺序分配**: 从 1 开始递增分配
- **不重用**: 已使用的 FD 不会被重用，避免竞态条件

### FD 映射表结构

```cpp
// 主映射：FD -> 文件路径
std::unordered_map<int, std::string> fd_map_;

// 反向映射：文件路径 -> 主 FD
std::unordered_map<std::string, int> path_fd_map_;

// 引用计数：文件路径 -> 打开次数
std::unordered_map<std::string, int> path_ref_count_;

// BwtFS 专用映射
std::unordered_map<int, BwtFS::Node::bw_tree*> fd_tree_map_;
std::unordered_map<int, int> fd_to_memory_fd_map_;
```

### FD 生命周期

```
1. open() 调用
   ├── 分配新的 FD (next_fd_++)
   ├── 创建 FD 映射
   ├── 初始化引用计数
   └── 返回 FD 给调用者

2. 文件操作期间
   ├── 使用 FD 查找文件路径
   ├── 通过路径访问实际数据
   └── 维护文件状态

3. close() 调用
   ├── 减少引用计数
   ├── 清理 FD 相关映射
   ├── 如果引用计数为 0，清理资源
   └── FD 永久不重用
```

## ⚠️ 错误处理策略

### 错误分类

#### 1. 系统错误
- 文件系统空间不足
- 权限拒绝
- I/O 设备错误

#### 2. 逻辑错误
- 文件不存在
- 无效的文件描述符
- 路径格式错误

#### 3. 数据错误
- 文件数据损坏
- Token 验证失败
- COW 操作失败

### 错误处理模式

```cpp
try {
    // 核心操作
    operation();
} catch (const std::exception& e) {
    LOG_ERROR << "Operation failed: " << e.what();

    // 清理资源
    cleanup_resources();

    // 返回错误码
    return -1;
}
```

### 数据损坏检测

```cpp
// 检测数据损坏的迹象
if (current_token == "*" ||
    current_token.empty() ||
    current_token == "memory") {
    LOG_ERROR << "Data corruption detected!";
    return -1;
}
```

### 重复操作检测

```cpp
// 检测重复的 finalize 操作
if (finalizing_files.find(normalized_path) != finalizing_files.end()) {
    LOG_WARNING << "[close] Duplicate finalize detected!";
    return 0;
}
```

## 💾 内存管理

### MemoryFS 内存策略

1. **文件数据存储**: 使用 `std::vector<char>` 存储文件内容
2. **元数据管理**: 使用 unordered_map 提供高效查找
3. **内存释放**: 文件删除时自动释放相关内存

### BwtFS 内存管理

1. **COW 临时文件**: 在 MemoryFS 中创建临时存储
2. **大文件分块**: 超大文件采用分块读取，避免内存溢出
3. **缓存策略**: 热点数据保持在内存中提高访问速度

### 内存优化建议

```cpp
// 大文件分块处理示例
const size_t chunk_size = 1024 * 1024; // 1MB chunks
for (size_t offset = 0; offset < file_size; offset += chunk_size) {
    size_t read_size = std::min(chunk_size, file_size - offset);
    // 处理每个数据块
}
```

## 🔒 并发控制

### 线程安全机制

1. **原子操作**: 引用计数使用原子类型
2. **互斥锁**: 关键操作使用互斥锁保护
3. **无锁设计**: 尽量避免锁竞争，提高性能

### 竞态条件防护

```cpp
// FD 不重用策略防止竞态条件
int new_fd = next_fd_++;
fd_map_[new_fd] = normalized_path;

// 引用计数防止资源竞争
path_ref_count_[normalized_path]++;
```

### 死锁预防

1. **锁顺序**: 始终按相同顺序获取锁
2. **锁粒度**: 使用细粒度锁减少锁竞争
3. **超时机制**: 避免无限等待

## 🐛 调试指南

### 日志级别

1. **LOG_INFO**: 一般信息，记录正常操作流程
2. **LOG_WARNING**: 警告信息，记录潜在问题
3. **LOG_ERROR**: 错误信息，记录操作失败

### 关键调试点

```cpp
// FD 分配调试
LOG_DEBUG << "[open] Assigned FD: " << fd << " for path: " << normalized_path;

// COW 操作调试
LOG_DEBUG << "[write] COW operation - temp_fd: " << temp_fd
          << ", original_token: " << current_token;

// 数据完整性调试
LOG_DEBUG << "[close] Token verification - old: " << old_token
          << ", new: " << new_token;
```

### 常见问题诊断

1. **文件损坏**: 检查 token 验证和 COW 最终化
2. **性能问题**: 监控内存使用和 I/O 操作
3. **资源泄露**: 检查 FD 映射和引用计数
4. **竞态条件**: 查看并发操作的日志顺序
