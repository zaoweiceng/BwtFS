# BwtFS Go客户端示例

这是BwtFS隐私保护文件系统的Go客户端示例，支持文件上传、下载、删除和获取文件系统信息功能。

## 功能特性

- ✅ 获取文件系统信息
- ✅ 上传文件（支持分块上传）
- ✅ 下载文件
- ✅ 删除文件
- ✅ 支持大文件处理
- ✅ 命令行界面
- ✅ 进度显示
- ✅ 错误处理
- ✅ 并发上传

## 环境要求

- Go 1.15+ 

## 安装依赖

```bash
go get github.com/google/uuid
```

## 使用方法

### 命令格式

```bash
go run bwtfs_client.go <command> [options]
```

### 命令列表

| 命令 | 功能 | 选项 |
|------|------|------|
| `info` | 获取文件系统信息 | `-url` (可选) |
| `upload` | 上传文件 | `-url` (可选), `-file` (必填) |
| `download` | 下载文件 | `-url` (可选), `-token` (必填), `-output` (必填) |
| `delete` | 删除文件 | `-url` (可选), `-token` (必填) |

### 选项说明

- `-url URL`: BwtFS服务地址 (默认: http://localhost:9999)
- `-file FILE`: 要上传的文件路径（仅upload命令）
- `-token TOKEN`: 文件访问令牌（download和delete命令）
- `-output OUTPUT`: 下载文件的输出路径（仅download命令）

## 使用示例

### 1. 获取文件系统信息

```bash
go run bwtfs_client.go info
```

输出示例：
```
BwtFS文件系统信息：
总大小: 268435456 bytes (256.00 MB)
已用大小: 10485760 bytes (10.00 MB)
剩余大小: 257949696 bytes (246.00 MB)
```

### 2. 上传文件

```bash
go run bwtfs_client.go upload -file ./test.txt
```

输出示例：
```
上传文件: test.txt
文件大小: 1024 bytes
总分块数: 1
上传进度: 100.0% (1/1)
上传成功!
文件令牌: abcdef1234567890abcdef1234567890abcdef1234567890
```

### 3. 下载文件

```bash
go run bwtfs_client.go download -token abcdef1234567890abcdef1234567890abcdef1234567890 -output ./downloaded.txt
```

输出示例：
```
下载文件...
文件大小: 1024 bytes
下载进度: 100.0% (1024/1024)
下载成功!
文件保存到: ./downloaded.txt
```

### 4. 删除文件

```bash
go run bwtfs_client.go delete -token abcdef1234567890abcdef1234567890abcdef1234567890
```

输出示例：
```
删除成功!
```

## API参考

### 获取文件系统信息
- **方法**: GET
- **路径**: `/system_size`
- **返回**: JSON格式，包含文件系统总大小

- **方法**: GET
- **路径**: `/free_size`
- **返回**: JSON格式，包含文件系统剩余空间

### 上传文件
- **方法**: POST
- **路径**: `/upload`
- **头信息**:
  - `Content-Type`: application/octet-stream
  - `X-File-Id`: 文件唯一标识符
  - `X-Chunk-Index`: 分块索引
  - `X-Total-Chunks`: 总分块数
  - `X-File-Size`: 文件总大小
  - `X-File-Type`: 文件类型
- **返回**: JSON格式，包含上传状态和文件令牌

### 下载文件
- **方法**: GET
- **路径**: `/{token}`
- **返回**: 文件二进制流

### 删除文件
- **方法**: DELETE
- **路径**: `/delete/{token}`
- **返回**: JSON格式，包含删除状态

## 注意事项

1. 确保BwtFS服务正在运行
2. 上传大文件时会自动分块（1MB/块）
3. 下载文件时会显示进度条
4. 所有操作都会进行错误处理和状态反馈
5. 令牌是访问文件的唯一凭证，请妥善保存
6. 上传时使用并发处理，最多10个并发连接

## 代码结构

```
bwtfs_client.go
├── BwtFSClient 结构体
│   ├── NewBwtFSClient - 创建客户端
│   ├── GetSystemInfo - 获取文件系统信息
│   ├── UploadFile - 上传文件
│   ├── DownloadFile - 下载文件
│   └── DeleteFile - 删除文件
├── 数据结构
│   ├── SystemInfo - 文件系统信息
│   └── UploadResponse - 上传响应
└── main 函数 - 命令行参数解析和执行
```
