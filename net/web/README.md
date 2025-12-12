# BwtFS Web 前端界面

BwtFS 隐私保护文件系统的 Web 前端界面，提供直观易用的文件管理体验。

## 🌟 主要特性

### 📁 文件管理
- **拖拽上传**：支持文件拖拽上传，实时显示上传进度
- **分块传输**：大文件自动分块上传，确保传输稳定性
- **目录导航**：层级文件夹结构，支持导航
- **搜索功能**：递归搜索文件和文件夹，实时显示结果
- **批量操作**：支持重命名、移动、删除等批量文件操作

### 🔐 隐私保护
- **令牌访问**：所有文件操作基于生成的访问令牌
- **加密传输**：文件通过 RCA 加密后存储在 BwTree 结构中
- **反取证**：黑白节点混淆存储，防止访问模式分析
- **安全删除**：文件删除后完全不可恢复

### 🎨 用户体验
- **现代化界面**：参考在线网盘的设计风格
- **响应式设计**：完美支持桌面和移动设备
- **实时通知**：操作结果通过右上角通知展示
- **文件预览**：内置图片、PDF、文本文件预览功能
- **树状选择**：移动操作支持树状展开的文件夹选择

## 🚀 快速开始

### 环境要求
- Node.js 16.0 或更高版本
- pnpm 8.0 或更高版本
- 现代浏览器（Chrome 90+, Firefox 88+, Safari 14+）

### 安装依赖
```bash
pnpm install
```

### 启动开发服务器
```bash
pnpm start
```

应用将在 http://localhost:3001 启动。

### 构建生产版本
```bash
pnpm build
```

## 📖 详细文档

### 项目架构

#### 技术栈
- **前端框架**：React 19.2.1 + TypeScript 4.9.5
- **UI 组件库**：Element Plus 2.12.0
- **HTTP 客户端**：Axios 1.13.2
- **图标库**：Lucide React
- **Markdown 渲染**：React-markdown 10.1.0
- **构建工具**：Create React App (react-scripts 5.0.1)

#### 目录结构
```
src/
├── components/           # React 组件
│   ├── FileManager.tsx   # 主要文件管理界面
│   ├── Header.tsx        # 应用头部组件
│   ├── Notification.tsx  # 通知组件
│   └── FilePreview.tsx   # 文件预览组件
├── services/            # 服务层
│   ├── api.ts           # HTTP API 客户端
│   └── fileManager.ts   # 本地文件结构管理
├── types/               # TypeScript 类型定义
│   └── index.ts
├── App.tsx              # 主应用组件
└── App.css              # 应用样式
```

### API 接口

#### 系统信息
- `GET /system_size` - 获取文件系统信息
- `GET /free_size` - 获取可用空间

#### 文件操作
- `POST /upload` - 上传文件（支持分块）
- `GET /{token}` - 下载文件
- `DELETE /{token}` - 删除文件

### 核心功能

#### 文件上传流程
1. 用户选择文件或拖拽到上传区域
2. 文件被分块为 1MB 的小块
3. 使用 X-Chunk-Index、X-Total-Chunks 等头部信息上传
4. 上传完成后，文件存储在 BwtFS 中并生成访问令牌
5. 本地文件结构更新到 localStorage

#### 文件下载流程
1. 用户点击下载按钮
2. 使用访问令牌请求文件
3. 文件以 4KB 块流式传输
4. 浏览器触发文件下载

#### 文件预览支持
- **图片格式**：jpg, jpeg, png, gif, bmp, webp, svg, ico
- **文档格式**：pdf
- **文本格式**：txt, md, markdown, json, xml, csv, log, ini, config, yml, yaml
- **代码格式**：js, ts, html, css, sql, py, java, cpp, c, h, hpp, sh, bat, ps1

#### 移动功能
- 树状展开的文件夹选择界面
- 支持展开/折叠子文件夹
- 智能过滤，防止无效移动操作
- 路径验证和错误处理

## ⚙️ 配置

### 环境变量
在 `.env` 文件中配置：
```env
REACT_APP_API_BASE_URL=http://127.0.0.1:9999
```

### 后端集成
确保 BwtFS 后端服务运行在配置的地址和端口上，并启用 CORS 支持。

## 🐛 故障排除

### 常见问题

#### 网络连接错误
- 确保后端服务正在运行
- 检查 `REACT_APP_API_BASE_URL` 配置
- 验证 CORS 设置

#### 文件上传失败
- 检查文件大小限制
- 确保网络连接稳定
- 查看浏览器控制台错误信息

#### PDF 预览问题
- 清除浏览器缓存
- 检查 PDF 文件是否损坏
- 确保浏览器支持 PDF 预览

## 🔗 相关链接

- [BwtFS 主项目](../../README.md)
- [后端服务](../README.md)
- [API 文档](../docs/api.md)

---

**BwtFS Web 前端** - 让隐私保护文件系统触手可及 🚀