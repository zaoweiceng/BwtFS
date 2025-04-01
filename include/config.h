#ifndef CONFIG_H
#define CONFIG_H
#include <cstddef>
#include <cstdint>
#include <string>
namespace BwtFS{
    const uint8_t VERSION = 0;         // 文件系统版本
    const size_t KB = 1024;            // 1KB
    const size_t MB = 1024 * KB;       // 1MB
    const size_t GB = 1024 * MB;       // 1GB
    const size_t TB = 1024 * GB;       // 1TB
    const size_t UNIT = KB;            // 块大小的单位
    const uint32_t BLOCK_SIZE = 4*KB;  // 块大小
    typedef uint32_t node_size;        // 节点内部大小：4KB，采用32位整型以提高性能


    namespace SIZE{
        const size_t __SIZE_OF_VERSION = sizeof(BwtFS::VERSION);            // 文件系统版本的大小
        const size_t __SIZE_OF_UNIT = sizeof(BwtFS::UNIT);                  // 块大小的单位的大小
        const size_t __SIZE_OF_BLOCK_SIZE = sizeof(BwtFS::BLOCK_SIZE);      // 块大小的大小
    };

    namespace DefaultConfig{
        // log
        const std::string LOG_LEVEL = "INFO";               // 日志级别
        const std::string LOG_PATH = "./bwtfs.log";         // 日志路径
        const bool LOG_TO_FILE = false;                     // 是否输出到文件
        const bool LOG_TO_CONSOLE = true;                   // 是否输出到控制台
        // config
        const std::string CONFIG_PATH = "./bwtfs.ini";      // 配置文件路径


    };
        
    
}
#endif