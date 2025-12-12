# BwtFS 网络服务

BwtFS Net 是 BwtFS 隐私保护文件系统的网络化扩展，提供现代化的 Web API 接口和用户界面。

## 🌟 主要特性

### 🖥️ Web 前端界面
- **现代化设计**：参考 Google Drive 的用户界面风格
- **响应式布局**：完美支持桌面和移动设备
- **实时通知**：操作结果通过右上角通知展示
- **文件预览**：支持图片、PDF、文本文件在线预览
- **拖拽上传**：直观的文件拖拽上传体验

### 📁 文件管理
- **分块上传**：大文件自动分块（1MB），确保传输稳定性
- **目录导航**：层级文件夹结构，支持面包屑导航
- **搜索功能**：递归搜索文件和文件夹，实时显示结果
- **批量操作**：重命名、移动、删除等批量文件操作
- **树状选择**：移动操作支持树状展开的文件夹选择

### 🔐 隐私保护
- **令牌访问**：所有文件操作基于生成的访问令牌
- **加密传输**：文件通过 RCA 加密后存储在 BwTree 结构中
- **反取证**：黑白节点混淆存储，防止访问模式分析
- **安全删除**：文件删除后完全不可恢复

### 🌐 RESTful API
- **HTTP 服务器**：基于 Boost.Beat 的高性能异步服务
- **CORS 支持**：完整的跨域资源共享支持
- **错误处理**：完善的错误响应机制
- **分块传输**：支持大文件分块上传和流式下载

## 🚀 快速开始

### 编译要求
- C++17 或更高版本
- CMake 3.10 或更高版本
- Boost 库（Beast, Asio, Json, Filesystem）
- Node.js 16.0+（前端开发）

### 编译后端服务
```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译项目
cmake --build .
```

### 启动服务
```bash
# 从构建目录运行
./bwtfs_net

# 默认监听端口 9999
# 端口可通过配置文件进行修改，具体请参考配置文档
```

### 启动前端
```bash
cd web

# 安装依赖
pnpm install

# 启动开发服务器
pnpm start

# 生产环境构建
pnpm build
```

### 访问应用
- **后端 API**：http://127.0.0.1:9999
- **Web 界面**：http://localhost:3001

## 📖 API 接口

### 系统信息
- **GET** `/system_size` - 获取文件系统信息
- **GET** `/free_size` - 获取可用空间

### 文件操作
- **POST** `/upload` - 上传文件（支持分块）
- **GET** `/{token}` - 下载文件
- **DELETE** `/{token}` - 删除文件

### 上传接口详情

**请求头**：
```
X-File-Id: 唯一文件标识符
X-Chunk-Index: 当前块索引（从0开始）
X-Total-Chunks: 总块数
X-File-Size: 文件总大小
X-File-Name: 文件名称
```

**成功响应**：
```json
{
    "success": true,
    "token": "generated_access_token",
    "message": "Upload completed"
}
```

## 📁 项目结构

```
net/
├── main.cpp              # 程序入口点
├── server.hpp            # HTTP 服务器实现
├── CMakeLists.txt        # CMake 构建配置
└── web/                  # React 前端项目
    ├── public/          # 静态资源
    │   ├── index.html     # HTML 模板
    │   └── favicon.ico    # 应用图标
    ├── src/             # 源代码
    │   ├── components/   # React 组件
    │   │   ├── FileManager.tsx
    │   │   ├── Header.tsx
    │   │   ├── Notification.tsx
    │   │   └── FilePreview.tsx
    │   ├── services/     # 服务层
    │   │   ├── api.ts
    │   │   └── fileManager.ts
    │   └── types/         # 类型定义
    ├── package.json     # 依赖配置
    └── README.md        # 前端详细文档
```

## 🛠️ 技术架构

### 后端技术栈
- **C++17**：现代 C++ 标准
- **Boost.Beast**：HTTP/1.1 服务器框架
- **Boost.Asio**：异步 I/O 库
- **Boost.JSON**：JSON 处理
- **BwtFS Core**：核心文件系统库

### 前端技术栈
- **React 19.2.1**：现代化前端框架
- **TypeScript 4.9.5**：类型安全的 JavaScript
- **Element Plus 2.12.0**：UI 组件库
- **Axios 1.13.2**：HTTP 客户端
- **React-markdown 10.1.0**：Markdown 渲染

## 🔧 配置选项

### 服务器配置
```ini
[server]
address = 127.0.0.1
port = 9999
max_body_size = 104857600
```

### 前端配置
```env
REACT_APP_API_BASE_URL=http://127.0.0.1:9999
```

## 📊 核心工作流程

### 文件上传流程
1. 用户选择文件或拖拽到上传区域
2. 文件被分块为 1MB 的小块
3. 通过 HTTP API 上传到后端
4. 文件存储在 BwtFS 中并生成访问令牌
5. 本地文件结构更新到 localStorage

### 文件预览支持
- **图片格式**：jpg, jpeg, png, gif, bmp, webp, svg, ico
- **文档格式**：pdf
- **文本格式**：txt, md, markdown, json, xml, csv, log, ini, config, yml, yaml
- **代码格式**：js, ts, html, css, sql, py, java, cpp, c, h, hpp, sh, bat, ps1

### 隐私保护机制
- **令牌认证**：所有文件操作需要有效令牌
- **数据加密**：RCA 加密算法保护数据安全
- **反取证存储**：黑白节点混淆防止访问分析
- **安全删除**：数据删除后完全不可恢复

## 🛠️ 使用指南

### JavaScript 客户端上传示例

```javascript
// 大文件分块上传
async function uploadLargeFile(file) {
    const CHUNK_SIZE = 1024 * 1024; // 1MB
    const totalChunks = Math.ceil(file.size / CHUNK_SIZE);
    const fileId = Date.now() + '-' + Math.random().toString(36).substr(2, 9);

    try {
        for (let chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
            const start = chunkIndex * CHUNK_SIZE;
            const end = Math.min(start + CHUNK_SIZE, file.size);
            const chunk = file.slice(start, end);

            const response = await fetch('http://127.0.0.1:9999/upload', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/octet-stream',
                    'X-File-Id': fileId,
                    'X-Chunk-Index': chunkIndex,
                    'X-Total-Chunks': totalChunks,
                    'X-File-Size': file.size,
                    'X-File-Name': file.name
                },
                body: chunk
            });

            // 最后一块返回访问令牌
            if (chunkIndex + 1 === totalChunks) {
                const data = await response.json();
                console.log('上传成功，访问令牌:', data.token);
                return data.token;
            }
        }
    } catch (error) {
        console.error('上传失败:', error);
    }
}
```

### Python 客户端示例

```python
import requests

# 上传文件
def upload_file(file_path):
    url = 'http://127.0.0.1:9999/upload'

    with open(file_path, 'rb') as f:
        files = {'file': f}
        data = {
            'X-File-Id': 'unique_file_id',
            'X-Chunk-Index': 0,
            'X-Total-Chunks': 1,
            'X-File-Size': os.path.getsize(file_path)
        }

        response = requests.post(url, files=files, headers=data)

        if response.status_code == 200:
            result = response.json()
            print(f"上传成功，令牌: {result['token']}")
            return result['token']

    return None

# 下载文件
def download_file(token, save_path):
    url = f'http://127.0.0.1:9999/{token}'

    response = requests.get(url)

    if response.status_code == 200:
        with open(save_path, 'wb') as f:
            f.write(response.content)
        print(f"下载成功: {save_path}")
        return True

    return False
```

## 🔧 开发指南

### 前端开发
```bash
cd web

# 安装依赖
pnpm install

# 启动开发服务器（带调试信息）
DISABLE_ESLINT_PLUGIN=true pnpm start

# 生产构建
pnpm build
```

### 添加新功能
1. 在 `src/components/` 中创建新组件
2. 在 `src/services/api.ts` 中添加 API 调用
3. 在 `src/types/` 中定义类型
4. 更新样式文件


## 🔗 相关链接

- [Web 前端详细文档](web/README.md)
- [BwtFS 主项目](../README.md)
- [技术文档](../docs/)