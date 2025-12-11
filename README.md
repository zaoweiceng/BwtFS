# BwtFS: Privacy-Preserving File System

[**🌏 Chinese**](README.zh-CN.md) | [**Development doc**](README_DEV.md) | [**Config**](README_CONFIG.md)

> 🚀 **Next-Generation Privacy-Preserving File System** - Featuring Black-White Tree Structure and RCA Encryption for Anti-Tracking Storage and Non-Recoverable Access

## ✨ Core Features

### 🔒 Privacy Protection
- **Anti-Tracking Storage**: Black-White Tree layered structure with random data distribution that prevents access pattern tracing
- **Non-Recoverable Access**: Token mechanism + multi-layer encryption ensures deleted data is completely unrecoverable
- **Secure Memory**: Memory data remains encrypted at all times, preventing memory dump attacks

### ⚡ High-Performance Architecture
- **Hybrid Storage**: Small files stored in memory + large files encrypted, with intelligent optimal strategy selection
- **COW Mechanism**: Copy-on-Write protects data integrity while supporting concurrent access
- **Asynchronous Processing**: High-performance async I/O based on Boost.Asio

### 🌐 Multiple Access Methods
- **FUSE Mount**: Cross-platform filesystem mounting (Windows/macOS/Linux)
- **Command Line Tools**: Interactive and batch operation modes
- **HTTP Service**: RESTful API + Web management interface

## 🏗️ Technical Architecture

```
┌─────────────────────────────────────────────────────┐
│                 User Interface Layer                  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
│  │ FUSE Mount  │  │ CMD Tools   │  │ HTTP Service│ │
│  └─────────────┘  └─────────────┘  └─────────────┘ │
├─────────────────────────────────────────────────────┤
│                  BwtFS Core Engine                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
│  │ B&W Tree    │  │ RCA Crypto  │  │ Token Access│ │
│  │ Storage     │  │             │  │ Control     │ │
│  └─────────────┘  └─────────────┘  └─────────────┘ │
├─────────────────────────────────────────────────────┤
│                Storage Management Layer                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐ │
│  │ Bitmap      │  │ Wear Level  │  │ Transaction │ │
│  │ Management  │  │ Balancing   │  │ Mechanism   │ │
│  └─────────────┘  └─────────────┘  └─────────────┘ │
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
- `bwtfs_net` - HTTP service (if enabled)

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

#### 3. HTTP Service

```bash
# Start HTTP server
./bwtfs_net

# Access Web interface
Open browser: http://localhost:9999

# API upload file
curl -X POST -F "file=@document.pdf" http://localhost:9999/upload

# API download file
curl -O http://localhost:9999/abc123def456...
```

## 📖 Documentation

| Document | Description |
|----------|-------------|
| [README_DEV.md](README_DEV.md) | Complete development documentation with architecture design and API documentation |
| [README_CONFIG.md](README_CONFIG.md) | Detailed configuration file documentation |
| [fs/README.md](fs/README.md) | FUSE subproject documentation |
| [net/README.md](net/README.md) | HTTP service documentation |
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
# System file size in bytes (default 512MB)
# size = 536870912

[server]
# HTTP service configuration
address = 127.0.0.1
port = 9999
max_body_size = 104857600
```

## 🎯 Use Cases

### Enterprise Environment
- **Multi-user File Sharing**: Employee file privacy protection, preventing administrator snooping
- **Confidential Document Management**: Compliant file storage meeting data protection regulations
- **R&D Code Protection**: Source code privacy storage preventing intellectual property leakage

### Personal Users
- **Private File Backup**: Secure backup of personal sensitive data
- **Temporary File Processing**: Secure handling of temporary sensitive data
- **Cross-device Transfer**: Encrypted file secure transfer between different devices

### Special Industries
- **Government Agencies**: Secure storage and transmission of confidential files
- **Financial Institutions**: Customer data and transaction record privacy protection
- **Healthcare Organizations**: Patient privacy data security management
- **Legal Industry**: Case materials and evidence file privacy protection

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
- **Small Files**: Stored in memory for fast access
- **Large Files**: Stored in BwtFS with encryption protection
- **Intelligent Selection**: Automatically selects optimal storage strategy based on file characteristics

## 🛡️ Security Features

- **Zero-Knowledge Architecture**: System administrators cannot access user file content
- **Forward Security**: Deleted files cannot be recovered by any technical means
- **Memory Protection**: Memory data remains encrypted at all times, preventing memory dump attacks
- **Transmission Encryption**: Network transmission uses HTTPS, preventing man-in-the-middle attacks
- **Access Auditing**: Complete operation logging and audit trails

## 🤝 Contributing

Welcome to submit Issues and Pull Requests!

### Development Environment Setup
1. Fork this project
2. Create feature branch: `git checkout -b feature/AmazingFeature`
3. Commit changes: `git commit -m 'Add some AmazingFeature'`
4. Push branch: `git push origin feature/AmazingFeature`
5. Open Pull Request

### Code Standards
- Use C++20 standard
- Follow existing code style
- Add necessary comments and documentation
- Ensure all tests pass

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

Thanks to the following open source projects:
- [Boost.Beast](https://github.com/boostorg/beast) - HTTP/WebSocket library
- [WinFSP](https://github.com/winfsp/winfsp) - Windows FUSE implementation
- [macFUSE](https://github.com/osxfuse/osxfuse) - macOS FUSE implementation
- [libfuse](https://github.com/libfuse/libfuse) - Linux FUSE implementation

---

⭐ If this project helps you, please give us a star!

📧 Contact us: [zaoweiceng@example.com](mailto:zaoweiceng@example.com)

🔗 Project homepage: [https://github.com/zaoweiceng/BwtFS](https://github.com/zaoweiceng/BwtFS)