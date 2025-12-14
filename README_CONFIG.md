# BwtFS 配置文件说明

## 配置文件位置

BwtFS 使用 `bwtfs.ini` 作为配置文件。默认情况下，程序会在当前目录查找此配置文件。如果没有该文件，则所有配置采用默认配置。

## 配置文件格式

配置文件采用 INI 格式，包含以下部分：

### [logging] - 日志配置

| 配置项 | 说明 | 默认值 | 可选值 |
|--------|------|--------|--------|
| log_level | 日志级别 | INFO | DEBUG, INFO, WARN, ERROR |
| log_path | 日志文件路径 | ./bwtfs.log | 任意有效的文件路径 |
| log_to_file | 是否输出日志到文件 | false | true, false |
| log_to_console | 是否输出日志到控制台 | true | true, false |

### [system] - 系统文件配置

**该配置项为可选配置，正常使用不需要配置**

| 配置项 | 说明 | 默认值 | 说明 |
|--------|------|--------|------|
| path | BwtFS 系统文件路径 | ./bwtfs.bwt | BwtFS 文件系统的存储文件路径 |
| size | 系统文件大小（字节） | 536870912 (512MB) | 最小 67108864 (64MB) |
| prefix | 系统文件前缀 | "" (空字符串) | 前缀文件 |
| filesystem_structure_json | 文件系统目录项 | ./filesystem_structure.json| 用于加载文件系统目录结构 |

### [server] - 服务器配置（用于 net 子项目）

> 注意：此部分配置主要用于对象存储服务

| 配置项 | 说明 | 默认值 | 说明 |
|--------|------|--------|------|
| host | 服务器监听地址 | 127.0.0.1 | 服务器绑定的 IP 地址 |
| port | 服务器监听端口 | 9999 | 服务器监听的端口号 |
| max_body_size | 最大请求体大小（字节） | 104857600 (100MB) | 客户端单次请求的最大数据量 |

## 配置文件示例

```ini
# BwtFS 配置文件示例

[logging]
# 日志级别: DEBUG, INFO, WARN, ERROR
log_level = INFO
# 日志文件路径
log_path = ./bwtfs.log
# 是否将日志输出到文件
log_to_file = false
# 是否将日志输出到控制台
log_to_console = true

[system]
# BwtFS 系统文件路径，使用对象存储服务是需要配置文件系统路径
# path = ./bwtfs.bwt
# 系统文件大小（字节），默认 512MB
# size = 536870912
# 系统文件前缀（可选）
# prefix =
# 文件系统目录项
# filesystem_structure_json = ./filesystem_structure.json

[server]
# 对象存储服务监听地址
host = 127.0.0.1
# 对象存储服务监听端口
port = 9999
# 最大请求体大小（字节），默认 100MB
max_body_size = 104857600
```

## 注意事项

1. **系统文件大小**：系统文件的最小大小为 64MB，建议设置为 512MB 或更大以获得更好的性能。
2. **服务器配置**：server 部分的配置仅在使用 net 子项目提供对象存储服务时生效。
3. **日志路径**：确保日志文件路径具有写入权限。
4. **配置修改**：修改配置后需要重启 BwtFS 服务才能生效。