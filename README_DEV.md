# BwtFS 开发文档

## 📋 目录

- [1. include 详细说明](#1-include-详细说明)
- [2. src 详细说明](#2-src-详细说明)
- [3. 核心项目介绍](#3-核心项目介绍)
- [4. fs 子项目介绍](#4-fs-子项目介绍)
- [5. net 子项目介绍](#5-net-子项目介绍)
- [6. cmd 子项目介绍](#6-cmd-子项目介绍)

---

## 1. include 详细说明

include 目录包含了 BwtFS 项目的核心接口定义和类声明，采用模块化设计，分为文件系统层、节点层和工具层。

### 1.1 文件系统层 (file/)

#### config.h - 系统配置
定义了 BwtFS 文件系统的核心常量和默认配置：

```cpp
namespace BwtFS {
    const uint8_t VERSION = 0;         // 文件系统版本
    const size_t BLOCK_SIZE = 4*KB;    // 块大小 4KB
    typedef unsigned node_size;        // 节点内部大小类型

    namespace DefaultConfig {
        const std::string LOG_LEVEL = "INFO";
        const std::string SYSTEM_FILE_PATH = "./bwtfs.bwt";
        const size_t SYSTEM_FILE_SIZE = 512 * MB;  // 默认512MB
    }
}
```

#### system.h - 文件系统核心接口
定义了 `FileSystem` 类，提供文件系统的主要功能：

- **读写操作**: `read()`, `write()` - 按块读写数据
- **系统信息**: `getVersion()`, `getFileSize()`, `getBlockSize()`
- **空间管理**: `getFreeSize()`, `getFilesSize()`
- **完整性检查**: `check()` - 验证文件系统完整性

**关键特性**：
- 使用读写锁 (`std::shared_mutex`) 保证并发安全
- 维护文件系统元数据（版本、大小、时间等）
- 集成位图管理器进行块分配

#### system_file.h - 物理文件抽象
`File` 类提供底层文件操作：

- **文件结构**: `[prefix(可选)] + [data] + [prefix_size]`
- **读写操作**: 支持按索引和大小读写
- **文件创建**: `createFile()` - 创建指定大小的文件
- **前缀支持**: 支持文件前缀用于扩展功能

#### bitmap.h - 位图管理
`Bitmap` 类管理文件系统块的分配状态：

- **块管理**: `set()`, `clear()`, `get()` - 设置块状态
- **分配策略**: `getFreeBlock()` - 获取空闲块，支持磨损均衡
- **磨损管理**: 维护块访问次数，实现磨损均衡算法
- **BPM 缓冲**: 位图页管理器提高访问效率

### 1.2 节点层 (node/)

BwtFS 使用黑白树结构实现数据的分层加密存储。

#### binary.h - 二进制数据封装
`Binary` 类提供统一的二进制数据操作：

- **数据格式**: 支持 BINARY、ASCII、BASE64 三种格式
- **内存管理**: 使用 `std::shared_ptr<std::vector<std::byte>>` 管理数据
- **操作接口**: 读写、追加、格式转换等丰富操作
- **运算符重载**: 支持 `==`, `!=`, `[]`, `+`, `^` 等操作

#### entry.h - 树节点条目
定义了黑白树的条目结构：

```cpp
class entry {
    size_t bitmap;        // 位图位置
    NodeType type;        // 节点类型 (WHITE_NODE/BLACK_NODE)
    uint16_t start;       // 起始位置
    uint16_t length;      // 长度
    uint16_t seed;        // 随机数种子
    uint8_t level;        // 加密层级
};
```

- **entry_list**: 条目列表管理器，支持序列化和随机化
- **固定大小**: 每个条目占用固定空间，提高查找效率

#### bw_node.h - 黑白树节点
定义了模板化的节点基类和具体实现：

```cpp
template<typename E>
class white_node : public tree_base_node<E> {
    // 白节点：存储实际文件数据
};

template<typename E>
class black_node : public tree_base_node<E> {
    // 黑节点：存储白节点的索引信息
};
```

- **加密支持**: 通过模板参数支持不同加密算法，主要使用可逆细胞自动机
- **节点类型**: 白节点存储数据，黑节点存储索引
- **序列化**: 支持二进制序列化和反序列化

#### bt_tree.h - 黑白树实现
核心的 `bw_tree` 类实现数据的分层存储：

```cpp
class bw_tree {
    // 写入流程: 数据 -> 白节点 -> 黑节点 -> 生成token
    void write(char* data, size_t size);

    // 读取流程: token -> 黑节点 -> 白节点 -> 数据
    Binary read(size_t index, size_t size);

    // 生成访问token
    std::string generate_tree();
};
```

**关键组件**：
- **TransactionWriter**: 事务写入器，支持回滚
- **TreeDataReader**: 树数据读取器，支持加密解密
- **内存池**: 使用内存池管理节点数据
- **线程池**: 异步处理提高性能

### 1.3 工具层 (util/)

#### log.h - 日志系统
提供线程安全的日志功能：

```cpp
// 日志级别
enum LogLevel { DEBUG, INFO, WARNING, ERROR_ };

// 使用宏简化日志记录
#define LOG_INFO LOG(LogLevel::INFO)
#define LOG_ERROR LOG(LogLevel::ERROR_)
```

- **单例模式**: 全局唯一的日志实例
- **多输出**: 支持控制台和文件同时输出
- **线程安全**: 使用互斥锁保护并发写入

#### ini_parser.h - 配置解析
`Config` 类管理 INI 格式的配置文件：

```cpp
class Config {
    // 单例获取
    static Config& getInstance();

    // 配置操作
    std::string get(const std::string& section, const std::string& key);
    void set(const std::string& section, const std::string& key, const std::string& value);
};
```

- **默认配置**: 内置完整的默认配置
- **自动创建**: 配置文件不存在时自动创建
- **动态加载**: 支持运行时重新加载配置

#### token.h - 访问令牌
实现文件的访问令牌机制：

```cpp
class Token {
    // 生成加密token
    std::string generate_token();

    // 从token解析信息
    void decode_token(const std::string& token);

    // 获取访问参数
    size_t get_bitmap();
    uint16_t get_start();
    uint16_t get_length();
};
```

- **RCA加密**: 使用 RCA 算法加密访问信息
- **Base64编码**: 生成 URL 安全的 token 字符串
- **信息保护**: token 中包含位置、大小、加密参数等信息

#### secure_ptr.h - 安全指针
提供内存加密保护机制：

```cpp
template <typename T>
class secure_ptr {
    // 安全访问对象
    EncryptedAccessProxy<T> operator->();
};

template <typename T, typename E>
static secure_ptr<T> make_secure(Args&&... args);
```

- **内存加密**: 内存中的数据始终处于加密状态
- **访问解密**: 访问时临时解密，访问后立即重新加密
- **RCA支持**: 集成 RCA 加密算法
- **RAII**: 自动管理内存生命周期

---

## 2. src 详细说明

src 目录包含了 include 中定义的类的具体实现。

### 2.1 核心实现文件

#### BwtFS.cpp - 主入口
简单的初始化入口函数：

```cpp
void init(){
    #ifdef _WIN32
    system("chcp 65001"); // Windows UTF-8 编码设置
    #endif
    LOG_CIALLO; // 可爱的启动日志
}
```

#### file/system.cpp - 文件系统实现
实现了文件系统的创建、初始化和管理：

**文件系统创建**:
```cpp
bool createBwtFS(const std::string& path, size_t file_size, const std::string& prefix) {
    // 1. 创建物理文件
    BwtFS::System::File::createFile(path, file_size, prefix);

    // 2. 初始化系统元数据
    // - 版本、大小、块信息
    // - 位图位置和大小
    // - 创建和修改时间

    // 3. 计算校验值
    std::hash<std::string> hash_fn;
    size_t string_hash_value = hash_fn(data.to_hex_string());

    // 4. 应用 RCA 加密
    BwtFS::Util::RCA cell(seed_of_cell, data);
    cell.forward();
}
```

**文件系统打开**:
```cpp
std::shared_ptr<FileSystem> openBwtFS(const std::string& path) {
    // 1. 验证文件存在性
    // 2. 读取认证信息
    // 3. 解密系统信息
    // 4. 验证完整性
    // 5. 创建文件系统对象
}
```

#### file/system_file.cpp - 物理文件操作
实现了底层文件的读写和管理：

**文件创建流程**:
1. **验证大小**: 最小 64MB 限制
2. **处理前缀**: 可选的前缀文件附加
3. **随机初始化**: 使用随机数据填充文件
4. **写入元数据**: 在文件末尾写入前缀大小

**读写操作**:
- **地址计算**: 考虑前缀大小的地址映射
- **边界检查**: 严格的读写边界验证
- **错误处理**: 完善的异常处理机制

### 2.2 实现特点

#### 安全性设计
- **数据加密**: 系统元数据使用 RCA 加密保护
- **完整性校验**: 哈希值验证数据完整性
- **访问控制**: 通过 token 机制控制文件访问

#### 性能优化
- **内存映射**: 使用内存缓冲区提高 I/O 效率
- **并发控制**: 读写锁支持高并发访问
- **磨损均衡**: 位图管理支持块级别的磨损均衡

#### 错误处理
- **分层验证**: 多层次的数据验证机制
- **异常安全**: 强异常安全保证
- **资源清理**: RAII 模式自动资源管理

---

## 3. 核心项目介绍

BwtFS 是一个创新的隐私保护文件系统，采用多层加密和抗追踪技术保护用户隐私。

### 3.1 核心特性

#### 反追踪存储 (Anti-Tracing Storage)
- **黑白树结构**: 数据分层存储，白节点存储文件内容，黑节点存储索引
- **随机分布**: 数据块在物理存储中随机分布，无法追踪访问模式
- **多层加密**: 支持 RCA、XOR 等多种加密算法

#### 不可恢复访问 (Non-Recoverable Access)
- **令牌机制**: 文件访问需要正确的令牌，令牌丢失则数据无法恢复
- **安全内存**: 内存数据始终加密，防止内存转储攻击
- **彻底删除**: 删除操作真正擦除数据，无法恢复恢复

#### 透明协议层 (Protocol-Layer Transparency)
- **无需修改**: 上层应用无需修改即可使用
- **标准接口**: 提供标准的文件操作接口
- **多种访问方式**: 支持 FUSE 挂载、命令行工具、HTTP 服务

### 3.2 技术架构

#### 存储架构
```
用户文件
    ↓
黑白树 (BwTree)
    ├── 白节点: 存储实际数据
    └── 黑节点: 存储索引信息
        ↓
文件系统 (FileSystem)
    ├── 位图管理 (Bitmap)
    ├── 物理文件 (File)
    └── 元数据管理
        ↓
物理存储
```

#### 加密层次
1. **节点级加密**: 每个节点使用独立的加密参数
2. **数据级加密**: 文件内容使用 RCA 算法加密
3. **内存级加密**: 安全指针保护内存数据

### 3.3 关键算法

#### RCA (Random Cellular Automata) 加密
基于元胞自动机的随机加密算法：
- **前向加密**: 数据写入时进行加密
- **后向解密**: 数据读取时进行解密
- **随机种子**: 每次操作使用不同的随机种子

#### 磨损均衡算法
```cpp
uint8_t getWearBlock(const size_t index) const {
    // 获取块访问次数
    // 选择磨损最轻的空闲块
    // 更新磨损计数
}
```

#### Token 生成算法
```cpp
std::string generate_token() {
    // 1. 收集访问参数 (位置、大小、加密信息)
    // 2. 应用 RCA 加密
    // 3. Base64 编码为 URL 字符串
}
```

---

## 4. fs 子项目介绍

fs 子项目实现了 FUSE (Filesystem in Userspace) 接口，允许将 BwtFS 挂载为标准文件系统。

### 4.1 混合存储架构

采用创新的混合存储策略，根据文件特性自动选择存储后端：

| 文件类型 | 存储后端 | 大小阈值 | 特性 |
|---------|---------|----------|------|
| 系统临时文件 | MemoryFS | < 100KB | 快速访问，内存存储 |
| 用户文件 | BwtFS | > 100KB | 加密存储，隐私保护 |

### 4.2 核心组件

#### BwtFSMounter 类
统一的文件系统挂载器，管理 MemoryFS 和 BwtFS：

```cpp
class BwtFSMounter {
private:
    SystemManager system_manager_;          // BwtFS 系统管理
    FileManager file_manager_;              // 文件元数据管理
    MemoryFS memory_fs_;                    // 内存文件系统

    // 文件描述符管理
    std::unordered_map<int, std::string> fd_map_;
    std::unordered_map<std::string, int> path_ref_count_;
};
```

#### 关键设计决策
- **独立 FD 分配**: 每个 open() 获得独立文件描述符
- **COW 机制**: Copy-on-Write 保护数据完整性
- **智能分类**: 自动识别文件类型选择存储策略
- **引用计数**: 正确管理并发访问

### 4.3 COW 机制

Copy-on-Write 机制确保并发安全和数据完整性：

```
写入流程:
1. 打开文件 → 创建临时文件
2. 读取原文件内容到临时文件
3. 所有写入操作指向临时文件
4. 关闭文件 → 将临时文件写入 BwtFS
5. 更新文件 Token
```

### 4.4 跨平台支持

支持三大操作系统的 FUSE 实现：
- **Windows**: WinFSP (Windows File System Proxy)
- **macOS**: macFUSE (formerly OSXFUSE)
- **Linux**: libfuse3

### 4.5 使用示例

```bash
# 创建 BwtFS 系统文件
./bwtfs_cmd create demo.bwt 256

# 挂载到 X 盘 (Windows)
./bwtfs_mount.exe X: ./demo.bwt ./fs.json

# 正常使用文件系统
echo "Hello BwtFS" > X:\test.txt
copy X:\test.txt .\
```

---

## 5. net 子项目介绍

net 子项目提供了基于 HTTP 的文件存储服务，支持 Web 界面和 RESTful API。

### 5.1 功能特性

- **文件上传**: 支持大文件分块上传，带进度显示
- **文件下载**: 基于 Token 的安全文件下载
- **文件删除**: 安全的文件删除操作
- **Web 界面**: 现代化的响应式管理界面
- **RESTful API**: 简洁的 HTTP API 接口

### 5.2 技术架构

#### 核心技术栈
- **网络库**: httplib.h (HTTP 服务器)
- **异步 I/O**: httplib.h 内置异步模型
- **JSON 处理**: jsoncpp
- **文件系统**: BwtFS 核心库

#### 架构设计
```
HTTP Client
    ↓
HTTP Server (httplib.h)
    ├── 请求路由
    ├── 分块处理
    ├── Token 管理
    └── BwtFS 集成
        ↓
BwtFS Storage
```

### 5.3 API 接口

#### 文件上传
```http
POST /upload
Headers:
- Content-Type: multipart/form-data
- X-File-Id: 唯一文件标识
- X-Chunk-Index: 数据块索引
- X-Total-Chunks: 总块数

Response:
{
    "success": true,
    "token": "bwtfs_file_token_here",
    "message": "Upload completed"
}
```

#### 文件下载
```http
GET /{token}

Response:
- 成功: 文件二进制流
- 失败: 404 Not Found
```

#### 文件删除
```http
DELETE /delete/{token}

Response:
{
    "success": true,
    "message": "File deleted successfully"
}
```

### 5.4 Web 界面

内置完整的 Web 管理界面：
- **响应式设计**: 支持桌面和移动端
- **AJAX 操作**: 无刷新文件操作
- **进度显示**: 实时上传下载进度
- **错误处理**: 友好的错误提示

### 5.5 性能优化

#### 连接管理
- **异步处理**: 基于 httplib.h 内置的异步 I/O
- **短连接**: 除文件上传外使用短连接
- **连接池**: 复用 TCP 连接提高效率

#### 内存管理
- **分块传输**: 1MB 上传块，4KB 下载块
- **缓冲区复用**: 10MB 固定缓冲区
- **及时清理**: 操作完成后立即释放资源

### 5.6 使用示例

```javascript
// JavaScript 文件上传
async function uploadFile(file) {
    const CHUNK_SIZE = 1024 * 1024; // 1MB chunks

    for (let chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
        const chunk = file.slice(start, end);

        const response = await fetch('http://localhost:9999/upload', {
            method: 'POST',
            headers: {
                'X-File-Id': fileId,
                'X-Chunk-Index': chunkIndex,
                'X-Total-Chunks': totalChunks
            },
            body: chunk
        });

        if (chunkIndex + 1 === totalChunks) {
            const data = await response.json();
            return data.token; // 返回文件访问 Token
        }
    }
}
```

---

## 6. cmd 子项目介绍

cmd 子项目提供了 BwtFS 的命令行接口，支持交互式和批处理操作模式。

### 6.1 命令架构

#### 模块化设计
```cpp
BwtFS::Command::CommandHandler
├── 参数解析 (parseArguments)
├── 命令执行 (executeCommand)
├── 交互模式 (runInteractiveMode)
└── 具体命令实现
    ├── 创建文件系统 (runCreateFSMode)
    ├── 写入文件 (runWriteFileMode)
    ├── 读取文件 (runRetrieveFileMode)
    └── 删除文件 (runDeleteFileMode)
```

#### 支持的命令类型
```cpp
enum class CommandType {
    INTERACTIVE,    // 交互模式
    CREATE_FS,      // 创建文件系统
    WRITE_FILE,     // 写入文件
    RETRIEVE_FILE,  // 获取文件
    DELETE_FILE,    // 删除文件
    SHOW_HELP,      // 显示帮助
    SHOW_VERSION,   // 显示版本
    SHOW_INFO,      // 显示信息
    UNKNOWN         // 未知命令
};
```

### 6.2 功能模块

#### CommandHandler (命令处理器)
```cpp
class CommandHandler {
public:
    // 解析命令行参数
    CommandArgs parseArguments(int argc, char* argv[]);

    // 执行命令
    int executeCommand(const CommandArgs& args);

    // 交互模式
    int runInteractiveMode();

private:
    // 具体命令实现
    int runCreateFSMode();
    int runWriteFileMode(const std::string& systemPath, const std::string& filePath);
    int runRetrieveFileMode(const std::string& systemPath, const std::string& token);
    int runDeleteFileMode(const std::string& systemPath, const std::string& token);
};
```

#### FileOps (文件操作)
```cpp
namespace FileOps {
    // 文件操作结果
    struct OperationResult {
        bool success;
        std::string message;
        std::string token;          // 写入操作返回的 token
        size_t bytesProcessed;      // 处理的字节数
    };

    // 核心操作
    OperationResult createBwtFS(const std::string& systemPath, size_t sizeMB);
    OperationResult writeFileToBwtFS(const std::string& systemPath, const std::string& filePath);
    OperationResult retrieveFileFromBwtFS(const std::string& systemPath, const std::string& token);
    OperationResult deleteFileFromBwtFS(const std::string& systemPath, const std::string& token);

    // 系统信息
    BwtFSInfo getBwtFSInfo(const std::string& systemPath);
}
```

#### UIHelper (用户界面辅助)
```cpp
namespace UI {
    // 用户交互
    void pause();                    // 等待用户按键
    void clearScreen();              // 清屏
    void showBanner();               // 显示启动横幅
    void showProgress(size_t current, size_t total); // 进度显示

    // 输入验证
    std::string getValidPath(const std::string& prompt);
    size_t getValidSize(const std::string& prompt);
}
```

### 6.3 使用示例

#### 交互模式
```bash
./bwtfs_cmd
# 进入交互模式，根据菜单选择操作

# 1. 创建文件系统
请输入文件系统路径: ./test.bwt
请输入文件系统大小 (MB): 256
✓ 文件系统创建成功

# 2. 写入文件
请输入文件系统路径: ./test.bwt
请输入要写入的文件路径: ./document.pdf
✓ 文件写入成功，Token: abc123def456...

# 3. 读取文件
请输入文件系统路径: ./test.bwt
请输入文件 Token: abc123def456...
请输入输出文件路径 (留空输出到控制台): ./output.pdf
✓ 文件读取成功
```

#### 命令行模式
```bash
# 创建文件系统
./bwtfs_cmd create ./test.bwt 256

# 写入文件
./bwtfs_cmd write ./test.bwt ./document.pdf

# 读取文件
./bwtfs_cmd retrieve ./test.bwt abc123def456... ./output.pdf

# 删除文件
./bwtfs_cmd delete ./test.bwt abc123def456...

# 查看系统信息
./bwtfs_cmd info ./test.bwt
```

---

## 总结

BwtFS 是一个功能完整的隐私保护文件系统，通过模块化设计实现了：

1. **核心存储引擎**: 黑白树结构 + RCA 加密 + Token 访问控制
2. **FUSE 文件系统**: 跨平台挂载 + 混合存储 + COW 机制
3. **HTTP 服务**: Web 界面 + RESTful API + 分块传输
4. **命令行工具**: 交互模式 + 批处理 + 用户友好界面

项目采用现代 C++20 标准，具有良好的可扩展性和维护性，为用户隐私保护提供了完整的解决方案。