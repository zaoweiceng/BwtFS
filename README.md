# BwtFS: Privacy-Preserving File System

[**🌏 Chinese**](README.zh-CN.md) | [**Development doc**](README_DEV.md) | [**Config**](README_CONFIG.md)

> 🚀 **Next-Generation Privacy-Preserving File System** - Featuring Black-White Tree Structure and RCA Encryption for Anti-Tracking Storage and Non-Recoverable Access

## ✨ Core Features

### 🔒 Privacy Protection
- **Anti-Tracking Storage**: Black-White Tree layered structure with random data distribution that prevents access pattern tracing
- **Non-Recoverable Access**: Token mechanism + multi-layer encryption ensures deleted data is completely unrecoverable
- **Secure Memory**: Memory data remains encrypted at all times, preventing memory dump

### ⚡ High-Performance Architecture
- **Hybrid Storage**: intelligent optimal strategy selection of memory and bwtfs
- **COW Mechanism**: Copy-on-Write protects data integrity while supporting concurrent access

### 🌐 Multiple Access Methods
- **FUSE Mount**: Cross-platform filesystem mounting (Windows/macOS/Linux)
- **Command Line Tools**: Interactive and batch operation modes
- **HTTP Service**: RESTful API + Modern Web management interface

## 🏗️ Technical Architecture

```
┌─────────────────────────────────────────────────────┐
│                 User Interface Layer                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ FUSE Mount  │  │ CMD Tools   │  │ HTTP Service│  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
├─────────────────────────────────────────────────────┤
│                  BwtFS Core Engine                  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ B&W Tree    │  │ RCA Crypto  │  │ Token Access│  │
│  │ Storage     │  │             │  │ Control     │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
├─────────────────────────────────────────────────────┤
│                Storage Management Layer             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ Bitmap      │  │ Wear Level  │  │ Transaction │  │
│  │ Management  │  │ Balancing   │  │ Mechanism   │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
└─────────────────────────────────────────────────────┘
```


## 🚀 Quick Start

### System Requirements

**Development Environment:**
- C++20 or higher
- CMake 3.10+
- Git

**Runtime Dependencies:**
- Windows: WinFSP runtime
- macOS: macFUSE 2.9+
- Linux: libfuse3

**Web Interface (Optional):**
- Node.js 16.0+ (for development)
- pnpm 8.0+ (package manager)
- Modern browser (Chrome 90+, Firefox 88+, Safari 14+)

### Build & Install

```bash
# Clone the project
git clone https://github.com/zaoweiceng/BwtFS.git
cd BwtFS

# Create build directory
mkdir build && cd build

# Configure project
cmake ..

# Build (Linux/macOS)
make

# Build (Windows)
cmake --build .
```

After compilation, executables are located in `build/bin/` directory:
- `bwtfs_cmd` - Command line tools
- `bwtfs_mount` - FUSE mount tool
- `bwtfs_net` - HTTP service

### Web Interface Setup

```bash
# Navigate to web directory
cd net/web

# Install dependencies with pnpm
pnpm install

# Start development server
pnpm start

# Production build
pnpm build

# Access web interface
# http://localhost:3000
```

**🚀 Complete startup process:**
1. Start the backend service: `./build/bwtfs_net` (port 9999)
2. Start the front-end interface: `cd net/web && pnpm start` (port 3000)
3. visit: http://localhost:3000

### Basic Usage

#### 1. Command Line Tools

```bash
# Create filesystem (256MB)
./bwtfs_cmd create ./myfs.bwt 256

# Write file
./bwtfs_cmd write ./myfs.bwt ./document.pdf
# Output: ✓ File written successfully, Token: abc123def456...

# Read file
./bwtfs_cmd retrieve ./myfs.bwt abc123def456... ./output.pdf

# Delete file
./bwtfs_cmd delete ./myfs.bwt abc123def456...

# View system information
./bwtfs_cmd info ./myfs.bwt
```

#### 2. FUSE Mount

```bash
# Mount to X: drive (Windows)
./bwtfs_mount.exe X: ./myfs.bwt ./fs.json

# Mount to mount point (Linux/macOS)
./bwtfs_mount ./mountpoint ./myfs.bwt ./fs.json

# Now use it like a regular filesystem
echo "Hello BwtFS" > X:\test.txt
copy X:\test.txt .\

# Unmount (Linux/macOS)
umount ./mountpoint
# Windows just exit the program
```

#### 3. HTTP Service & Web Interface

```bash
# Start HTTP server (from build directory)
./bwtfs_net

# Access Modern Web interface
Open browser: http://localhost:3000

# Access API directly
API Base URL: http://127.0.0.1:9999

# API download file
curl -O http://127.0.0.1:9999/abc123def456...
```

**Web Interface Features:**
- 🎨 **Modern-style UI** - Modern, responsive design
- 📁 **File Management** - Drag-drop upload, folder operations, search
- 🔍 **File Preview** - Built-in preview for images, PDF, text, markdown
- 🔐 **Privacy-focused** - Token-based access with secure operations
- ⚡ **Real-time Updates** - Toast notifications and live status
- 📱 **Mobile Ready** - Responsive design for all devices

## 📖 Documentation

| Document | Description |
|----------|-------------|
| [README_DEV.md](README_DEV.md) | Complete development documentation with architecture design and API documentation |
| [README_CONFIG.md](README_CONFIG.md) | Detailed configuration file documentation |
| [net/README.md](net/README.md) | BwtFS 网络服务完整文档 - 包含后端 API 和前端集成 |
| [net/web/README.md](net/web/README.md) | Web 前端界面详细文档 - React 技术栈和功能说明 |
| [fs/README.md](fs/README.md) | FUSE subproject documentation |
| [fs/README_DEV.md](fs/README_DEV.md) | FUSE development documentation |
| [net/README_DEV.md](net/README_DEV.md) | HTTP service development documentation |

## 🔧 Configuration

BwtFS uses `bwtfs.ini` configuration file:

```ini
[logging]
log_level = INFO
log_path = ./bwtfs.log
log_to_file = false
log_to_console = true

[system]
# BwtFS system file path (optional, default ./bwtfs.bwt)
# path = ./bwtfs.bwt

[server]
# HTTP service configuration
address = 127.0.0.1
port = 9999
max_body_size = 104857600
```

## 🎯 Use Cases

### Personal Users
- **Private File Backup**: Secure backup of personal sensitive data
- **Temporary File Processing**: Secure handling of temporary sensitive data
- **Cross-device Transfer**: Encrypted file secure transfer between different devices

## 🔬 Technical Highlights

### RCA Encryption Algorithm
Random Cellular Automata based encryption algorithm:
- **Forward Encryption**: Multi-level encryption during data writing
- **Backward Decryption**: Step-by-step decryption during data reading
- **Random Seeds**: Each node uses independent random seeds

### Black-White Tree Structure
Innovative layered storage structure:
- **White Nodes**: Store actual file data
- **Black Nodes**: Store index information of white nodes
- **Random Distribution**: Nodes are randomly distributed in storage space

### Token Access Control
- **Unique Identifier**: Each file generates a unique access token
- **Information Encapsulation**: Token contains position, size, encryption parameters and other information
- **Access Control**: Files cannot be accessed without correct token

### Hybrid Storage Strategy
- **Temp Files**: Stored in memory for fast access
- **User Files**: Stored in BwtFS with encryption protection
- **Intelligent Selection**: Automatically selects optimal storage strategy based on file characteristics


## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

Thanks to the following open source projects:

**Core System:**
- [WinFSP](https://github.com/winfsp/winfsp) - Windows FUSE implementation
- [macFUSE](https://github.com/osxfuse/osxfuse) - macOS FUSE implementation
- [libfuse](https://github.com/libfuse/libfuse) - Linux FUSE implementation
- [cpp-httplib](https://github.com/yhirose/cpp-httplib) - Http framework
- [nlohmann/json](https://github.com/nlohmann/json) - Json library

**Web Frontend:**
- [React](https://github.com/facebook/react) - Modern UI framework
- [Element Plus](https://github.com/element-plus/element-plus) - Vue 3 UI components
- [TypeScript](https://github.com/microsoft/TypeScript) - Type-safe JavaScript
- [Axios](https://github.com/axios/axios) - HTTP client library
- [Lucide React](https://github.com/lucide-icons/lucide) - Icon library
- [React-markdown](https://github.com/remarkjs/react-markdown) - Markdown renderer

---

⭐ If this project helps you, please give us a star!