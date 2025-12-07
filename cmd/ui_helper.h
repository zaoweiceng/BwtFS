#ifndef UI_HELPER_H
#define UI_HELPER_H

#include <string>
#include <vector>
#include <iostream>

namespace BwtFS {
    namespace UI {

    void showVersion();
    void showHelp();
    void showBanner();
    std::string promptFileSystemPath(const std::string& prompt = "请输入文件系统路径：");
    std::string promptFilePath(const std::string& prompt = "请输入要写入的文件路径：");
    size_t promptFileSystemSize(size_t defaultSizeMB, const std::string& prompt = "");
    void showSuccess(const std::string& operation, const std::string& detail = "");
    void showError(const std::string& operation, const std::string& detail = "");
    void showWarning(const std::string& message);
    void showInfo(const std::string& message);
    void showProgress(const std::string& message);
    bool confirm(const std::string& message);
    void pause();
    void clearScreen();
    void showSeparator(int length = 50, char character = '-');
    bool isValidBwtFSPath(const std::string& path);
    std::string formatFileSize(size_t bytes);
    void showFileSummary(const std::string& filePath, size_t fileSize);
    std::string promptToken(const std::string& prompt = "请输入访问令牌：");
    std::string promptOutputPath(const std::string& prompt = "请输入输出文件路径（直接回车输出到标准输出）：");

    } // namespace UI
} // namespace BwtFS

#endif // UI_HELPER_H