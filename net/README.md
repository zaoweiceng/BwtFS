# BwtFS Net - HTTP 服务器接口

BwtFS Net 是 BwtFS 文件系统的 HTTP 服务器子项目，提供了基于 Web 的文件存储和访问接口。

## 功能特性

- **文件上传**：支持大文件分块上传，带有进度显示和断点续传功能
- **文件下载**：基于 Token 的安全文件下载
- **文件删除**：安全的文件删除操作
- **Web 界面**：现代化的响应式 Web 管理界面
- **RESTful API**：简洁的 HTTP API 接口

## 快速开始

### 编译项目

```bash
mkdir build && cd build
cmake ..
make
```

可执行文件编译完成之后在`build/bin`目录下

### 运行服务器

```bash
# 使用默认配置文件
./bwtfs_net

# 启动服务
./bwtfs_net

# 查看帮助信息
./bwtfs_net --help
```

启动服务前需要配置文件系统路径，具体参考配置说明文档

### 访问服务

启动服务后，在浏览器中访问：
```
http://localhost:9999
```

## API 接口

### 1. 文件上传

**接口**：`POST /upload`

**说明**：上传文件到 BwtFS，支持大文件分块上传

**请求格式**：
- Content-Type: `multipart/form-data`
- 支持文件分块上传（每块 1MB）

**响应**：
```json
{
    "success": true,
    "token": "bwtfs_file_token_here",
    "message": "Upload completed"
}
```

### 2. 文件下载

**接口**：`GET /{token}`

**说明**：使用 Token 下载文件

**响应**：
- 成功：文件二进制流
- 失败：404 Not Found

### 3. 文件删除

**接口**：`DELETE /delete/{token}`

**说明**：删除指定 Token 对应的文件

**响应**：
```json
{
    "success": true,
    "message": "File deleted successfully"
}
```

### 4. Web 界面

**接口**：`GET /`

**说明**：访问 Web 管理界面，提供文件上传、下载和删除功能

## 配置选项

服务器配置通过 `bwtfs.ini` 文件的 `[server]` 部分设置：

```ini
[server]
# 服务器监听地址
address = 127.0.0.1
# 服务器监听端口
port = 9999
# 最大请求体大小（字节）
max_body_size = 104857600
```

## 技术架构

- **网络库**：Boost.Beast (HTTP 服务器)
- **异步 I/O**：Boost.Asio
- **JSON 处理**：Boost.JSON
- **文件系统**：BwtFS 核心库


## 使用示例

对于大文件，需要分块上传。建议使用 JavaScript 客户端或编写脚本进行分块上传。

### 使用 JavaScript 上传文件

```javascript
async function uploadFile(file) {
    if (!file) {
        console.error("Please select a file to upload.");
        return;
    }

    const CHUNK_SIZE = 1024 * 1024; // 1MB chunks
    const totalChunks = Math.ceil(file.size / CHUNK_SIZE);
    const fileId = Date.now() + '-' + Math.random().toString(36).substr(2, 9);

    try {
        for (let chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
            const start = chunkIndex * CHUNK_SIZE;
            const end = Math.min(start + CHUNK_SIZE, file.size);
            const chunk = file.slice(start, end);

            const response = await fetch('http://localhost:9999/upload', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/octet-stream',
                    'X-File-Id': fileId,
                    'Connection': 'keep-alive',
                    'X-Chunk-Index': chunkIndex,
                    'X-Total-Chunks': totalChunks,
                    'X-File-Size': file.size,
                    'X-File-Type': file.type
                },
                body: chunk
            });

            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }

            // Last chunk returns the token
            if (chunkIndex + 1 === totalChunks) {
                const data = await response.json();
                console.log('Upload successful. Token:', data.token);
                return data.token;
            }
        }
    } catch (error) {
        console.error('Error uploading file:', error);
    }
}

// 使用示例
const fileInput = document.querySelector('input[type="file"]');
fileInput.addEventListener('change', (e) => {
    if (e.target.files[0]) {
        uploadFile(e.target.files[0]);
    }
});
```