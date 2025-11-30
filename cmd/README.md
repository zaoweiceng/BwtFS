# BwtFS 命令行工具

## 🌟 特性

- **🔒 隐私保护**: 抗溯源存储，不可恢复数据访问
- **🚀 高性能**: 多线程处理，支持大文件操作
- **💻 用户友好**: 直观的交互界面和丰富的命令行选项
- **📊 实时反馈**: 进度显示、文件信息统计
- **🔑 令牌访问**: 基于令牌的安全文件检索系统
- **🛡️ 安全可靠**: 完善的错误处理和资源管理

## 📋 目录

- [安装](#安装)
- [快速开始](#快速开始)
- [命令行用法](#命令行用法)
- [交互模式](#交互模式)
- [命令选项](#命令选项)
- [示例](#示例)
- [故障排除](#故障排除)
- [开发指南](#开发指南)

## 🛠️ 安装

### 前置要求

- **C++17** 兼容编译器 (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake** 3.10 或更高版本
- **BwtFS** 核心库

### 构建步骤

```bash
# 从项目根目录构建
mkdir build
cd build
cmake ..
make

# 可执行文件位置
./bin/BWTFileSystemProject
```

## 🚀 快速开始

### 1. 创建文件系统

```bash
# 交互式创建
./BWTFileSystemProject create

# 查看创建结果
./BWTFileSystemProject --info ./data.bwt
```

### 2. 写入文件

```bash
# 写入文件到文件系统
./BWTFileSystemProject ./data.bwt ./document.pdf

# 自动识别路径（顺序不重要）
./BWTFileSystemProject ./photo.jpg ./storage.bwt
```

### 3. 使用令牌获取文件

```bash
# 输出到标准输出
./BWTFileSystemProject ./data.bwt abc123def456...

# 保存到指定文件
./BWTFileSystemProject ./data.bwt abc123def456... ./retrieved_file.pdf

# 使用重定向保存
./BWTFileSystemProject ./data.bwt abc123def456... > output.txt
```

### 4. 查看信息

```bash
# 显示文件系统信息
./BWTFileSystemProject --info ./data.bwt

# 显示版本信息
./BWTFileSystemProject --version
```

## 💻 命令行用法

### 基本语法

```bash
BWTFileSystemProject [选项] [命令] [参数...]
```

### 命令类型

#### 1. 交互模式（默认）
```bash
./BWTFileSystemProject
```
提供菜单驱动的交互界面。

#### 2. 创建文件系统
```bash
./BWTFileSystemProject create
```
引导用户创建新的 BwtFS 文件系统。

#### 3. 写入文件
```bash
./BWTFileSystemProject <filesystem_path> <file_path>
./BWTFileSystemProject <file_path> <filesystem_path>
```
将文件写入到 BwtFS 文件系统，写入成功后返回访问令牌。

#### 4. 使用令牌获取文件
```bash
./BWTFileSystemProject <filesystem_path> <token>
./BWTFileSystemProject <filesystem_path> <token> <output_path>
```
使用访问令牌从 BwtFS 文件系统中检索文件：
- 第一个参数：BwtFS 文件系统路径
- 第二个参数：文件访问令牌
- 第三个参数（可选）：输出文件路径，默认输出到标准输出

## ⚙️ 命令选项

### 基本选项

| 选项 | 简写 | 描述 |
|------|------|------|
| `--help` | `-h` | 显示帮助信息 |
| `--version` | `-v` | 显示版本信息 |
| `--verbose` | | 启用详细输出模式 |
| `--info <path>` | | 显示文件系统信息 |

### 使用示例

```bash
# 显示帮助
./BWTFileSystemProject --help

# 详细模式写入文件
./BWTFileSystemProject --verbose ./storage.bwt ./large_file.zip

# 使用令牌获取文件
./BWTFileSystemProject --verbose ./storage.bwt abc123def456... ./retrieved_file.zip

# 查看文件系统信息
./BWTFileSystemProject --info ./storage.bwt
```

## 🎮 交互模式

### 菜单选项

运行 `./BWTFileSystemProject` 进入交互模式：

```
请选择操作:
1. 写入文件到文件系统
2. 创建新的文件系统
3. 使用令牌获取文件
4. 显示文件系统信息
5. 退出程序
```

### 交互流程

1. **写入文件**
   - 选择选项 1
   - 输入 BwtFS 文件系统路径
   - 输入要写入的文件路径
   - 查看进度和结果
   - 保存生成的访问令牌

2. **创建文件系统**
   - 选择选项 2
   - 输入文件系统路径
   - 输入文件系统大小（MB）
   - 确认创建

3. **使用令牌获取文件**
   - 选择选项 3
   - 输入 BwtFS 文件系统路径
   - 输入文件访问令牌
   - 输入输出文件路径（可选，直接回车输出到标准输出）
   - 查看进度和结果

4. **查看信息**
   - 选择选项 4
   - 输入文件系统路径
   - 查看详细统计信息

## 📊 功能详解

### 文件系统信息

使用 `--info` 选项查看详细信息：

```bash
./BWTFileSystemProject --info ./data.bwt
```

输出示例：
```
文件系统路径: ./data.bwt
总大小: 1.00 GB
已使用: 256.50 MB
可用空间: 767.50 MB
块大小: 4.00 KB
创建时间: 2025-03-30 14:30:15
修改时间: 2025-03-30 15:45:22
使用率: 25.0%
```

### 进度显示

在详细模式下，文件写入和获取都会显示实时进度：

```
进度: 45.2% (450.5 MB/1.00 GB)
```

### 令牌系统

BwtFS 使用基于令牌的文件访问系统：

**令牌特点：**
- 每个文件写入时自动生成唯一令牌
- 令牌验证确保文件访问安全性

**令牌格式验证：**
- 不包含路径分隔符（如 '/'）

**令牌使用：**
- 写入文件后系统会返回访问令牌
- 保存令牌用于后续文件检索
- 令牌可以安全地分享给授权用户

### 文件大小格式化

工具自动将文件大小转换为合适的单位：
- B（字节）
- KB（千字节）
- MB（兆字节）
- GB（吉字节）
- TB（太字节）

## 📝 示例

### 示例：基本工作流程

```bash
# 1. 创建新的文件系统
./BWTFileSystemProject create
# 输入路径：./mydata.bwt
# 输入大小：1024

# 2. 写入文档
./BWTFileSystemProject ./mydata.bwt ./report.pdf
# 输出：文件写入成功，Token: abc123def456...

# 3. 使用令牌获取文件
./BWTFileSystemProject ./mydata.bwt abc123def456... ./retrieved_report.pdf

# 4. 查看结果
./BWTFileSystemProject --info ./mydata.bwt
```

## 🔧 故障排除

### 常见问题

#### 1. 文件系统打开失败
```
✗ 打开文件系统失败: ./data.bwt
```

**解决方案:**
- 检查文件是否存在
- 确认文件扩展名为 `.bwt`
- 验证文件权限

#### 2. 文件写入失败
```
✗ 写入文件失败: 无法打开文件
```

**解决方案:**
- 检查源文件是否存在
- 确认文件读取权限
- 验证文件系统空间

#### 3. 创建文件系统失败
```
✗ 创建文件系统失败: 文件已存在
```

**解决方案:**
- 选择不同的文件路径
- 确认是否覆盖现有文件
- 检查磁盘空间

#### 4. 令牌获取文件失败
```
✗ Token错误: 无效的访问令牌格式
```

**解决方案:**
- 检查令牌是否完整
- 确认令牌没有多余的空格或换行符
- 验证令牌是否来自正确的文件系统

#### 5. 文件不存在
```
✗ 获取文件失败: File not found
```

**解决方案:**
- 确认令牌对应的文件仍然存在于文件系统中
- 检查文件系统是否损坏
- 验证令牌是否正确复制

#### 6. 令牌验证失败
```
✗ Token错误: 令牌包含无效字符
```

**解决方案:**
- 重新复制令牌，确保没有额外字符
- 检查令牌中没有路径分隔符（如 '/'）

### 调试技巧

1. **启用详细模式**
   ```bash
   ./BWTFileSystemProject --verbose <其他参数>
   ```

2. **检查文件系统状态**
   ```bash
   ./BWTFileSystemProject --info <文件系统路径>
   ```

3. **验证文件权限**
   ```bash
   ls -la *.bwt
   ```


## 👨‍💻 开发指南

### 项目结构

```
cmd/
├── main.cpp              # 程序入口
├── command_handler.h/cpp # 命令处理逻辑
├── file_operations.h/cpp # 文件操作功能
├── ui_helper.h/cpp       # 用户界面辅助
├── CMakeLists.txt        # 构建配置
└── README.md            # 本文档
```

### 模块说明

#### CommandHandler
- 命令行参数解析
- 交互模式控制
- 错误处理和恢复

#### FileOperations
- 文件系统操作
- 文件读写处理
- 进度回调管理

#### UIHelper
- 用户界面显示
- 输入提示和验证
- 格式化和美化

### 编码规范

- 使用 C++17 标准
- 遵循 RAII 原则
- 完整的错误处理
- 详细的文档注释
