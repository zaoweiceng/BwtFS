#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <string>
#include <memory>
#include <functional>

namespace BwtFS {
    namespace FileOps {
    struct OperationResult {
        bool success;
        std::string message;
        std::string token;  // 用于文件写入操作
        size_t bytesProcessed = 0;

        OperationResult(bool s = false, const std::string& msg = "")
            : success(s), message(msg) {}
    };
    OperationResult writeFileToBwtFS(const std::string& systemPath, const std::string& filePath);
    OperationResult createBwtFS(const std::string& systemPath, size_t sizeMB);
    bool fileExists(const std::string& filePath);
    bool bwtFSExists(const std::string& systemPath);
    size_t getFileSize(const std::string& filePath);
    bool isFileReadable(const std::string& filePath);
    bool isValidBwtFSFile(const std::string& path);
    using ProgressCallback = std::function<void(size_t bytesWritten, size_t totalBytes)>;
    OperationResult writeFileToBwtFSWithProgress(
        const std::string& systemPath,
        const std::string& filePath,
        ProgressCallback progressCallback = nullptr
    );
    struct BwtFSInfo {
        size_t totalSize = 0;
        size_t usedSize = 0;
        size_t freeSize = 0;
        size_t blockSize = 0;
        std::string createTime;
        std::string modifyTime;
        bool isValid = false;
    };
    BwtFSInfo getBwtFSInfo(const std::string& systemPath);
    OperationResult retrieveFileFromBwtFS(const std::string& systemPath, const std::string& token, const std::string& outputPath);
    OperationResult retrieveFileFromBwtFS(const std::string& systemPath, const std::string& token);
    bool isValidToken(const std::string& token);
    OperationResult deleteFileFromBwtFS(const std::string& systemPath, const std::string& token);

    } // namespace FileOps
} // namespace BwtFS

#endif // FILE_OPERATIONS_H