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
- **HTTP服务**: RESTful API + 现代化Web管理界面

## 🏗️ 技术架构

```
┌─────────────────────────────────────────────────────┐
│                   用户接口层                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ FUSE 挂载    │  │ 命令行工具   │  │ HTTP 服务    │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
├─────────────────────────────────────────────────────┤
│                   BwtFS 核心引擎                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ 黑白树存储    │  │ RCA 加密    │  │ Token 访问   │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
├─────────────────────────────────────────────────────┤
│                   存储管理层                          │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ 位图管理     │  │ 磨损均衡     │   │ 事务机制     │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
└─────────────────────────────────────────────────────┘
```

### 🌐 Web 界面架构

```
┌─────────────────────────────────────────────────────┐
│                 Web 前端层                           │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ React 19.2  │  │ TypeScript  │  │ Element Plus│  │
│  │ 组件         │  │ 服务        │  │ UI 库       │   │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
├─────────────────────────────────────────────────────┤
│                 HTTP API 层                         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ RESTful API │  │ 分块上传     │  │ CORS 支持    │  │
│  │ 接口         │  │ 功能        │  │              │ │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
├─────────────────────────────────────────────────────┤
│                客户端存储层                           │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ LocalStorage│  │ 文件         │  │ 搜索和       │  │
│  │ 持久化       │  │ 结构管理      │  │ 导航功能     │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
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

**Web 界面 (可选):**
- Node.js 16.0+ (开发环境)
- pnpm 8.0+ (包管理器)
- 现代浏览器 (Chrome 90+, Firefox 88+, Safari 14+)

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

### Web 界面设置

```bash
# 进入 web 目录
cd net/web

# 使用 pnpm 安装依赖
pnpm install

# 启动开发服务器
pnpm start

# 生产环境构建
pnpm build

# 访问 web 界面
# http://localhost:3001
```

**🚀 完整启动流程:**
1. 启动后端服务：`./build/BWTFileSystemProject` (端口 9999)
2. 启动前端界面：`cd net/web && pnpm start` (端口 3001)
3. 访问：http://localhost:3001

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

#### 3. HTTP 服务和 Web 界面

```bash
# 启动 HTTP 服务器 (从构建目录/build/bin)
./bwtfs_net

# 访问现代化 Web 界面
浏览器打开: http://localhost:3001

# 直接访问 API
API 基础地址: http://127.0.0.1:9999

# API 上传文件 (支持大文件分块上传)
curl -X POST \
  -H "X-File-Id: unique_file_id" \
  -H "X-Chunk-Index: 0" \
  -H "X-Total-Chunks: 1" \
  -H "X-File-Size: $(stat -c%s document.pdf)" \
  -H "X-File-Name: document.pdf" \
  --data-binary @document.pdf \
  http://127.0.0.1:9999/upload

# API 下载文件
curl -O http://127.0.0.1:9999/abc123def456...
```

**Web 界面功能特性:**
- 🎨 **现代风格界面** - 现代化响应式设计
- 📁 **文件管理** - 拖拽上传、文件夹操作、搜索功能
- 🔍 **文件预览** - 内置图片、PDF、文本、Markdown 预览
- 🔐 **隐私保护** - 基于 Token 的安全操作
- ⚡ **实时更新** - 通知提示和实时状态
- 📱 **移动端支持** - 响应式设计适配所有设备

> 📖 **详细文档**: [网络服务文档](net/README.md) | [前端界面文档](net/web/README.md)

## 📖 详细文档

| 文档 | 描述 |
|------|------|
| [README_DEV.md](README_DEV.md) | 完整的开发文档，包含架构设计和API说明 |
| [README_CONFIG.md](README_CONFIG.md) | 配置文件详细说明 |
| [net/README.md](net/README.md) | BwtFS 网络服务完整文档 - 包含后端 API 和前端集成 |
| [net/web/README.md](net/web/README.md) | Web 前端界面详细文档 - React 技术栈和功能说明 |
| [fs/README.md](fs/README.md) | FUSE 子项目文档 |
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

**核心系统:**
- [Boost.Beast](https://github.com/boostorg/beast) - HTTP/WebSocket库
- [WinFSP](https://github.com/winfsp/winfsp) - Windows FUSE实现
- [macFUSE](https://github.com/osxfuse/osxfuse) - macOS FUSE实现
- [libfuse](https://github.com/libfuse/libfuse) - Linux FUSE实现

**Web 前端:**
- [React](https://github.com/facebook/react) - 现代化UI框架
- [Element Plus](https://github.com/element-plus/element-plus) - Vue 3 UI组件
- [TypeScript](https://github.com/microsoft/TypeScript) - 类型安全的JavaScript
- [Axios](https://github.com/axios/axios) - HTTP客户端库
- [Lucide React](https://github.com/lucide-icons/lucide) - 图标库
- [React-markdown](https://github.com/remarkjs/react-markdown) - Markdown渲染器

---

⭐ 如果这个项目对您有帮助，请给一个星标！

🔗 项目主页: [https://github.com/zaoweiceng/BwtFS](https://github.com/zaoweiceng/BwtFS)