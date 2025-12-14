# BwtFS - External Libraries

本文档记录了BwtFS项目中使用的外部第三方库。

## 目录结构
```
lib/
├── httplib.h/          # HTTP服务器库
└── jsoncpp/            # JSON解析库
```

## 1. httplib - HTTP服务器库

**项目地址**: https://github.com/yhirose/cpp-httplib

**版本**: 当前项目使用的是v0.11.3（根据include目录下的文件时间戳判断）

**用途**:
- 为BwtFS提供HTTP对象存储服务
- 处理文件上传和下载请求
- RESTful API接口实现

**主要特性**:
- 跨平台支持（Windows、Linux、macOS）
- 仅头文件实现，易于集成
- 支持HTTP/1.1
- 内置线程池
- 支持SSL/TLS（可选）

**使用方式**:
```cpp
#include "httplib.h"

httplib::Server svr;
svr.Post("/upload", [](const httplib::Request& req, httplib::Response& res) {
    // 处理文件上传
});
svr.listen("0.0.0.0", 8080);
```

**许可**: MIT License

## 2. jsoncpp - JSON解析库

**项目地址**: https://github.com/open-source-parsers/jsoncpp

**版本**: 当前项目使用的是v1.9.5（根据文件结构判断）

**用途**:
- 解析BwtFS配置文件（bwtfs.ini）
- 序列化和反序列化元数据
- 处理API请求和响应

**主要特性**:
- 高性能JSON解析器
- 支持JSON的读写
- 提供C++风格的API
- 完整的错误处理机制

**使用方式**:
```cpp
#include <json/json.h>

Json::Value root;
Json::Reader reader;
reader.parse(json_string, root);

// 访问JSON数据
std::string value = root["key"].asString();
```

**许可**: MIT License

## 库管理说明

### 当前状态
- 所有库都是源码形式直接包含在项目中
- 没有使用包管理器（如vcpkg、conan等）
- 库文件存放在lib目录下供项目使用

### 版本管理
- 建议定期更新库版本以获得安全修复和性能提升
- 更新前请确保兼容性测试通过

### 依赖关系
- httplib和jsoncpp都是独立库，相互之间没有依赖关系
- 两个库都是仅头文件实现，不需要额外的链接步骤

## 添加新库的指南

如果要为BwtFS项目添加新的外部库，请遵循以下步骤：

1. 将库文件放入lib目录下的对应子目录
2. 更新此README文档
3. 在CMakeLists.txt中添加相应的include路径
4. 确保库的许可证与BwtFS项目兼容
5. 更新项目的依赖文档

## 注意事项

1. **许可证兼容性**: 所有使用的库都采用MIT许可证，与BwtFS项目兼容
2. **版本锁定**: 建议在CI/CD中锁定版本，避免意外的版本更新
3. **安全更新**: 定期检查库的安全更新并及时应用
4. **跨平台支持**: 选择的库都支持多平台编译