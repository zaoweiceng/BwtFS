#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <string>
#include <memory>
#include <functional>

namespace BwtFS {
namespace FileOps {

/**
 * @brief 文件操作结果结构
 */
struct OperationResult {
    bool success;
    std::string message;
    std::string token;  // 用于文件写入操作
    size_t bytesProcessed = 0;

    OperationResult(bool s = false, const std::string& msg = "")
        : success(s), message(msg) {}
};

/**
 * @brief 写入文件到BwtFS文件系统
 * @param systemPath BwtFS文件系统路径
 * @param filePath 要写入的文件路径
 * @return 操作结果
 */
OperationResult writeFileToBwtFS(const std::string& systemPath, const std::string& filePath);

/**
 * @brief 创建新的BwtFS文件系统
 * @param systemPath 文件系统路径
 * @param sizeMB 文件系统大小（MB）
 * @return 操作结果
 */
OperationResult createBwtFS(const std::string& systemPath, size_t sizeMB);

/**
 * @brief 检查文件是否存在
 * @param filePath 文件路径
 * @return 文件是否存在
 */
bool fileExists(const std::string& filePath);

/**
 * @brief 检查BwtFS文件系统是否存在
 * @param systemPath 文件系统路径
 * @return 文件系统是否存在
 */
bool bwtFSExists(const std::string& systemPath);

/**
 * @brief 获取文件大小
 * @param filePath 文件路径
 * @return 文件大小（字节），失败返回0
 */
size_t getFileSize(const std::string& filePath);

/**
 * @brief 验证文件是否可读
 * @param filePath 文件路径
 * @return 文件是否可读
 */
bool isFileReadable(const std::string& filePath);

/**
 * @brief 验证路径是否为有效的BwtFS文件
 * @param path 路径
 * @return 是否为有效的BwtFS文件
 */
bool isValidBwtFSFile(const std::string& path);

/**
 * @brief 显示文件写入进度（回调函数类型）
 * @param bytesWritten 已写入字节数
 * @param totalBytes 总字节数
 */
using ProgressCallback = std::function<void(size_t bytesWritten, size_t totalBytes)>;

/**
 * @brief 带进度回调的文件写入操作
 * @param systemPath BwtFS文件系统路径
 * @param filePath 要写入的文件路径
 * @param progressCallback 进度回调函数
 * @return 操作结果
 */
OperationResult writeFileToBwtFSWithProgress(
    const std::string& systemPath,
    const std::string& filePath,
    ProgressCallback progressCallback = nullptr
);

/**
 * @brief 获取BwtFS文件系统信息
 */
struct BwtFSInfo {
    size_t totalSize = 0;
    size_t usedSize = 0;
    size_t freeSize = 0;
    size_t blockSize = 0;
    std::string createTime;
    std::string modifyTime;
    bool isValid = false;
};

/**
 * @brief 获取BwtFS文件系统信息
 * @param systemPath 文件系统路径
 * @return 文件系统信息
 */
BwtFSInfo getBwtFSInfo(const std::string& systemPath);

} // namespace FileOps
} // namespace BwtFS

#endif // FILE_OPERATIONS_H