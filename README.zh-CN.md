# BwtFS: 隐私保护文件系统

[**English**](README.md) | [**开发文档**](README_DEV.md) | [**配置说明**](README_CONFIG.md)

> 🚀 **新一代隐私保护文件系统** - 采用黑白树结构和RCA加密技术，实现反追踪存储和不可恢复访问

## ✨ 核心特性

### 🔒 隐私保护
- **反追踪存储**: 黑白树分层结构，数据随机分布，无法追踪访问模式
- **不可恢复访问**: Token机制 + 多层加密，数据删除后彻底无法恢复
- **安全内存**: 内存数据始终加密，防止内存转储攻击

### ⚡ 高性能架构
- **混合存储**: 小文件内存存储 + 大文件加密存储，智能选择最优策略
- **COW机制**: Copy-on-Write保护数据完整性，支持并发访问
- **异步处理**: 基于Boost.Asio的高性能异步I/O

### 🌐 多种访问方式
- **FUSE挂载**: 跨平台文件系统挂载 (Windows/macOS/Linux)
- **命令行工具**: 交互式和批处理操作模式
- **HTTP服务**: RESTful API + Web管理界面

## 🏗️ 技术架构

```
┌─────────────────────────────────────────────────────┐
│                   用户接口层                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
│  │ FUSE 挂载   │  │ 命令行工具  │  │ HTTP 服务   │ │
│  └─────────────┘  └─────────────┘  └─────────────┘ │
├─────────────────────────────────────────────────────┤
│                   BwtFS 核心引擎                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
│  │ 黑白树存储  │  │ RCA 加密    │  │ Token 访问  │ │
│  └─────────────┘  └─────────────┘  └─────────────┘ │
├─────────────────────────────────────────────────────┤
│                   存储管理层                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
│  │ 位图管理    │  │ 磨损均衡    │  │ 事务机制    │ │
│  └─────────────┘  └─────────────┘  └─────────────┘ │
└─────────────────────────────────────────────────────┘
```

## 🚀 快速开始

### 系统要求

**开发环境:**
- C++20 或更高版本
- CMake 3.10+
- Git

**运行时依赖:**
- Windows: WinFSP 运行时
- macOS: macFUSE 2.9+
- Linux: libfuse3

### 编译安装

```bash
# 克隆项目
git clone https://github.com/zaoweiceng/BwtFS.git
cd BwtFS

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译 (Linux/macOS)
make

# 编译 (Windows)
cmake --build .
```

编译完成后，可执行文件位于 `build/bin/` 目录：
- `bwtfs_cmd` - 命令行工具
- `bwtfs_mount` - FUSE挂载工具
- `bwtfs_net` - HTTP服务 (如果启用)

### 基本使用

#### 1. 命令行工具

```bash
# 创建文件系统 (256MB)
./bwtfs_cmd create ./myfs.bwt 256

# 写入文件
./bwtfs_cmd write ./myfs.bwt ./document.pdf
# 输出: ✓ 文件写入成功，Token: abc123def456...

# 读取文件
./bwtfs_cmd retrieve ./myfs.bwt abc123def456... ./output.pdf

# 删除文件
./bwtfs_cmd delete ./myfs.bwt abc123def456...

# 查看系统信息
./bwtfs_cmd info ./myfs.bwt
```

#### 2. FUSE 挂载

```bash
# 挂载到 X 盘 (Windows)
./bwtfs_mount.exe X: ./myfs.bwt ./fs.json

# 挂载到挂载点 (Linux/macOS)
./bwtfs_mount ./mountpoint ./myfs.bwt ./fs.json

# 现在可以像普通文件系统一样使用
echo "Hello BwtFS" > X:\test.txt
copy X:\test.txt .\

# 卸载 (Linux/macOS)
umount ./mountpoint
# Windows 直接退出程序即可
```

#### 3. HTTP 服务

```bash
# 启动 HTTP 服务器
./bwtfs_net

# 访问 Web 界面
浏览器打开: http://localhost:9999

# API 下载文件
curl -O http://localhost:9999/abc123def456...
```

## 📖 详细文档

| 文档 | 描述 |
|------|------|
| [README_DEV.md](README_DEV.md) | 完整的开发文档，包含架构设计和API说明 |
| [README_CONFIG.md](README_CONFIG.md) | 配置文件详细说明 |
| [fs/README.md](fs/README.md) | FUSE 子项目文档 |
| [net/README.md](net/README.md) | HTTP 服务文档 |
| [fs/README_DEV.md](fs/README_DEV.md) | FUSE 开发文档 |
| [net/README_DEV.md](net/README_DEV.md) | HTTP 服务开发文档 |

## 🔧 配置说明

BwtFS 使用 `bwtfs.ini` 配置文件：

```ini
[logging]
log_level = INFO
log_path = ./bwtfs.log
log_to_file = false
log_to_console = true

[system]
# BwtFS 系统文件路径 (可选，默认 ./bwtfs.bwt)
# path = ./bwtfs.bwt
# 系统文件大小 (字节，默认 512MB)
# size = 536870912

[server]
# HTTP 服务配置
address = 127.0.0.1
port = 9999
max_body_size = 104857600
```

## 🎯 应用场景

### 企业环境
- **多用户文件共享**: 员工文件隐私保护，防止管理员窥探
- **机密文档管理**: 合规文件存储，满足数据保护法规
- **研发代码保护**: 源代码隐私存储，防止知识产权泄露

### 个人用户
- **隐私文件备份**: 个人敏感信息安全备份
- **临时文件处理**: 安全处理临时敏感数据
- **跨设备传输**: 加密文件在不同设备间安全传输

### 特殊行业
- **政府机构**: 机密文件安全存储和传输
- **金融机构**: 客户数据和交易记录隐私保护
- **医疗机构**: 患者隐私数据安全管理
- **法律行业**: 案件材料和证据文件隐私保护

## 🔬 技术亮点

### RCA 加密算法
基于元胞自动机的随机加密算法：
- **前向加密**: 数据写入时进行多级加密
- **后向解密**: 数据读取时逐级解密
- **随机种子**: 每个节点使用独立随机种子

### 黑白树结构
创新的分层存储结构：
- **白节点**: 存储实际文件数据
- **黑节点**: 存储白节点的索引信息
- **随机分布**: 节点在存储空间中随机分布

### Token 访问控制
- **唯一标识**: 每个文件生成唯一访问Token
- **信息封装**: Token包含位置、大小、加密参数等信息
- **访问控制**: 无正确Token无法访问任何文件数据

### 混合存储策略
- **小文件**: 存储在内存中，快速访问
- **大文件**: 存储在BwtFS中，加密保护
- **智能选择**: 根据文件特性自动选择最优存储策略

## 🛡️ 安全特性

- **零知识架构**: 系统管理员无法访问用户文件内容
- **前向安全**: 已删除文件无法通过任何技术手段恢复
- **内存保护**: 内存数据始终加密，防止内存转储攻击
- **传输加密**: 网络传输使用HTTPS，防止中间人攻击
- **访问审计**: 完整的操作日志记录和审计追踪

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

感谢以下开源项目的支持：
- [Boost.Beast](https://github.com/boostorg/beast) - HTTP/WebSocket库
- [WinFSP](https://github.com/winfsp/winfsp) - Windows FUSE实现
- [macFUSE](https://github.com/osxfuse/osxfuse) - macOS FUSE实现
- [libfuse](https://github.com/libfuse/libfuse) - Linux FUSE实现

---

⭐ 如果这个项目对您有帮助，请给一个星标！

🔗 项目主页: [https://github.com/zaoweiceng/BwtFS](https://github.com/zaoweiceng/BwtFS)