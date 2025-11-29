#include "file_operations.h"
#include "ui_helper.h"
#include "BwtFS.h"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <functional>

namespace BwtFS {
namespace FileOps {

OperationResult writeFileToBwtFS(const std::string& systemPath, const std::string& filePath) {
    return writeFileToBwtFSWithProgress(systemPath, filePath, nullptr);
}

OperationResult writeFileToBwtFSWithProgress(
    const std::string& systemPath,
    const std::string& filePath,
    ProgressCallback progressCallback) {

    UI::showProgress("正在打开文件系统");

    // 打开BwtFS文件系统
    auto system = BwtFS::System::openBwtFS(systemPath);
    if (system == nullptr) {
        return OperationResult(false, "无法打开BwtFS文件系统: " + systemPath);
    }

    UI::showProgress("正在检查文件");

    // 检查文件是否存在且可读
    if (!fileExists(filePath)) {
        return OperationResult(false, "文件不存在: " + filePath);
    }

    if (!isFileReadable(filePath)) {
        return OperationResult(false, "文件不可读: " + filePath);
    }

    // 获取文件大小
    size_t fileSize = getFileSize(filePath);
    if (fileSize == 0) {
        return OperationResult(false, "文件为空: " + filePath);
    }

    UI::showInfo("文件大小: " + UI::formatFileSize(fileSize));

    // 打开要写入的文件
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return OperationResult(false, "无法打开文件: " + filePath);
    }

    UI::showProgress("正在写入数据到文件系统");

    try {
        // 创建黑白树
        BwtFS::Node::bw_tree tree;

        // 读取和写入缓冲区
        constexpr size_t BUFFER_SIZE = 8190;
        char buffer[BUFFER_SIZE];
        size_t totalBytesRead = 0;

        // 循环读取文件并写入黑白树
        while (true) {
            file.read(buffer, BUFFER_SIZE);
            size_t bytesRead = file.gcount();

            if (bytesRead == 0) {
                break;
            }

            tree.write(buffer, bytesRead);
            totalBytesRead += bytesRead;

            // 调用进度回调
            if (progressCallback) {
                progressCallback(totalBytesRead, fileSize);
            }
        }

        // 刷新缓冲区
        UI::showProgress("正在完成数据写入");
        tree.flush();
        file.close();

        // 等待树构建完成
        tree.join();

        // 获取访问令牌
        std::string token = tree.get_token();

        OperationResult result(true, "文件写入成功");
        result.bytesProcessed = totalBytesRead;
        result.token = token;

        return result;

    } catch (const std::exception& e) {
        return OperationResult(false, std::string("写入过程中发生错误: ") + e.what());
    }
}

OperationResult createBwtFS(const std::string& systemPath, size_t sizeMB) {
    try {
        UI::showProgress("正在创建文件系统文件");

        // 创建文件系统文件
        BwtFS::System::File::createFile(systemPath, sizeMB * BwtFS::MB);

        UI::showProgress("正在初始化文件系统");

        // 初始化文件系统
        if (!BwtFS::System::initBwtFS(systemPath)) {
            // 清理已创建的文件
            std::filesystem::remove(systemPath);
            return OperationResult(false, "无法初始化文件系统: " + systemPath);
        }

        std::string sizeStr = UI::formatFileSize(sizeMB);
        OperationResult result(true, "文件系统创建成功");
        result.message += "，大小: " + sizeStr;

        return result;

    } catch (const std::exception& e) {
        // 清理可能已创建的文件
        if (std::filesystem::exists(systemPath)) {
            std::filesystem::remove(systemPath);
        }
        return OperationResult(false, std::string("创建文件系统时发生错误: ") + e.what());
    }
}

bool fileExists(const std::string& filePath) {
    return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

bool bwtFSExists(const std::string& systemPath) {
    if (!fileExists(systemPath)) {
        return false;
    }

    // 检查文件扩展名
    return UI::isValidBwtFSPath(systemPath);
}

size_t getFileSize(const std::string& filePath) {
    try {
        return std::filesystem::file_size(filePath);
    } catch (const std::exception&) {
        return 0;
    }
}

bool isFileReadable(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    return file.is_open();
}

bool isValidBwtFSFile(const std::string& path) {
    if (!bwtFSExists(path)) {
        return false;
    }

    try {
        // 尝试打开文件系统
        auto system = BwtFS::System::openBwtFS(path);
        return system != nullptr;
    } catch (const std::exception&) {
        return false;
    }
}

BwtFSInfo getBwtFSInfo(const std::string& systemPath) {
    BwtFSInfo info;

    try {
        auto system = BwtFS::System::openBwtFS(systemPath);
        if (system == nullptr) {
            info.isValid = false;
            return info;
        }

        info.totalSize = system->getFileSize();
        info.blockSize = system->getBlockSize();
        info.usedSize = system->getFilesSize();
        info.freeSize = system->getFreeSize();
        info.isValid = true;

        // 转换时间戳为可读格式
        auto createTime = static_cast<time_t>(system->getCreateTime());
        auto modifyTime = static_cast<time_t>(system->getModifyTime());

        std::ostringstream ss;
        ss << std::put_time(std::localtime(&createTime), "%Y-%m-%d %H:%M:%S");
        info.createTime = ss.str();

        ss.str("");
        ss.clear();
        ss << std::put_time(std::localtime(&modifyTime), "%Y-%m-%d %H:%M:%S");
        info.modifyTime = ss.str();

    } catch (const std::exception& e) {
        info.isValid = false;
        // 可以将错误信息记录到日志
    }

    return info;
}

} // namespace FileOps
} // namespace BwtFS