# BwtFS Python客户端示例

这是BwtFS隐私保护文件系统的Python客户端示例，支持文件上传、下载、删除和获取文件系统信息功能。

## 功能特性

- ✅ 获取文件系统信息
- ✅ 上传文件（支持分块上传）
- ✅ 下载文件
- ✅ 删除文件
- ✅ 支持大文件处理
- ✅ 命令行界面
- ✅ 进度显示
- ✅ 错误处理

## 环境要求

- Python 3.6+ 
- requests 库
- tqdm 库（用于进度显示）

## 安装依赖

```bash
pip install requests tqdm
```

## 使用方法

### 命令格式

```bash
python bwtfs_client.py <command> [options]
```

### 命令列表

| 命令 | 功能 | 选项 |
|------|------|------|
| `info` | 获取文件系统信息 | `-u/--url` (可选) |
| `upload` | 上传文件 | `-u/--url` (可选), `-f/--file` (必填) |
| `download` | 下载文件 | `-u/--url` (可选), `-t/--token` (必填), `-o/--output` (必填) |
| `delete` | 删除文件 | `-u/--url` (可选), `-t/--token` (必填) |

### 选项说明

- `-h, --help`: 显示帮助信息
- `-u URL, --url URL`: BwtFS服务地址 (默认: http://localhost:9999)
- `-f FILE, --file FILE`: 要上传的文件路径（仅upload命令）
- `-t TOKEN, --token TOKEN`: 文件访问令牌（download和delete命令）
- `-o OUTPUT, --output OUTPUT`: 下载文件的输出路径（仅download命令）

## 使用示例

### 1. 获取文件系统信息

```bash
python bwtfs_client.py info
```

输出示例：
```
BwtFS文件系统信息：
总大小: 268,435,456 bytes (256.00 MB)
已用大小: 10,485,760 bytes (10.00 MB)
剩余大小: 257,949,696 bytes (246.00 MB)
```

### 2. 上传文件

```bash
python bwtfs_client.py upload -f ./test.txt
```

输出示例：
```
Uploading file: test.txt
File size: 1024 bytes
Total chunks: 1
100%|██████████████████████████████████████████████| 1/1 [00:00<00:00,  2.50chunk/s]
Upload successful!
File token: abcdef1234567890abcdef1234567890abcdef1234567890
```

### 3. 下载文件

```bash
python bwtfs_client.py download -t abcdef1234567890abcdef1234567890abcdef1234567890 -o ./downloaded.txt
```

输出示例：
```
Downloading file...
File size: 1024 bytes
100%|██████████████████████████████████████████████| 1.00k/1.00k [00:00<00:00, 2.00kB/s]
Download successful!
File saved to: ./downloaded.txt
```

### 4. 删除文件

```bash
python bwtfs_client.py delete -t abcdef1234567890abcdef1234567890abcdef1234567890
```

输出示例：
```
Delete successful!
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

## 代码结构

```
bwtfs_client.py
├── BwtFSClient类
│   ├── __init__ - 初始化客户端
│   ├── get_system_info - 获取文件系统信息
│   ├── upload_file - 上传文件
│   ├── download_file - 下载文件
│   └── delete_file - 删除文件
└── main函数 - 命令行参数解析和执行
```