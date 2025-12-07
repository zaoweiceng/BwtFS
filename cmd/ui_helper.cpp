#include "ui_helper.h"
#include "BwtFS.h"
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <limits>
#include <sstream>
#include "util/log.h"

using BwtFS::Util::Logger;

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace BwtFS {
    namespace UI {

    void showVersion() {
        showBanner();
        std::cout << "\n";
        showSeparator();
        std::cout << "版本: 1.0.0\n";
        std::cout << "编译时间: " << __DATE__ << " " << __TIME__ << "\n";
        std::cout << "C++标准: C++17\n";
        std::cout << "平台: " <<
    #ifdef _WIN32
            "Windows"
    #elif __linux__
            "Linux"
    #elif __APPLE__
            "macOS"
    #else
            "Unknown"
    #endif
            << "\n";
        showSeparator();
    }

    void showHelp() {
        showSeparator();
        std::cout << "BwtFS 隐私保护文件系统 - 使用帮助\n";
        showSeparator();
        std::cout << "\n使用方法:\n";
        std::cout << "  " << "BWTFileSystemProject" << "                    [交互模式]\n";
        std::cout << "  " << "BWTFileSystemProject" << " create              [创建文件系统]\n";
        std::cout << "  " << "BWTFileSystemProject" << " <fs_path> <file>    [写入文件到文件系统]\n";
        std::cout << "  " << "BWTFileSystemProject" << " <file> <fs_path>    [写入文件到文件系统]\n";
        std::cout << "  " << "BWTFileSystemProject" << " <fs_path> <token>  [使用令牌获取文件]\n";
        std::cout << "  " << "BWTFileSystemProject" << " <fs_path> <token> <output_path>  [获取文件并保存到指定路径]\n";
        std::cout << "  " << "BWTFileSystemProject" << " --help             [显示此帮助信息]\n";
        std::cout << "  " << "BWTFileSystemProject" << " --version          [显示版本信息]\n";
        std::cout << "  " << "BWTFileSystemProject" << " --info <fs_path>   [显示文件系统信息]\n";
        std::cout << "\n参数说明:\n";
        std::cout << "  <fs_path>    BwtFS文件系统路径（.bwt扩展名）\n";
        std::cout << "  <file>       要写入的文件路径\n";
        std::cout << "  <token>      文件访问令牌（写入时自动生成）\n";
        std::cout << "  <output_path> 输出文件路径（可选，默认输出到标准输出）\n";
        std::cout << "\n示例:\n";
        std::cout << "  " << "BWTFileSystemProject" << " create\n";
        std::cout << "  " << "BWTFileSystemProject" << " ./data.bwt ./document.pdf\n";
        std::cout << "  " << "BWTFileSystemProject" << " ./photo.jpg ./data.bwt\n";
        std::cout << "  " << "BWTFileSystemProject" << " ./data.bwt abc123def456...  # 输出到标准输出\n";
        std::cout << "  " << "BWTFileSystemProject" << " ./data.bwt abc123def456... ./retrieved.pdf\n";
        showSeparator();
    }

    void showBanner() {
        clearScreen();
        std::cout << "\n";
        showSeparator(45, '=');
        std::cout << "██████╗ ██╗    ██╗████████╗███████╗███████╗\n";
        std::cout << "██╔══██╗██║    ██║╚══██╔══╝██╔════╝██╔════╝\n";
        std::cout << "██████╔╝██║ █╗ ██║   ██║   █████╗  ███████╗\n";
        std::cout << "██╔══██╗██║███╗██║   ██║   ██╔══╝  ╚════██║\n";
        std::cout << "██████╔╝╚███╔███╔╝   ██║   ██╗     ███████║\n";
        std::cout << "╚═════╝  ╚══╝╚══╝    ╚═╝   ╚═╝     ╚══════╝\n";
        showSeparator(45, '=');
    }

    std::string promptFileSystemPath(const std::string& prompt) {
        std::string path;
        std::cout << prompt;
        std::getline(std::cin, path);

        // 去除首尾空白字符
        path.erase(0, path.find_first_not_of(" \t\n\r"));
        path.erase(path.find_last_not_of(" \t\n\r") + 1);

        if (path.empty()) {
            showError("输入错误", "文件系统路径不能为空");
            return "";
        }

        // 如果没有扩展名，自动添加.bwt
        if (path.find('.') == std::string::npos) {
            path += ".bwt";
        }

        return path;
    }

    std::string promptFilePath(const std::string& prompt) {
        std::string path;
        std::cout << prompt;
        std::getline(std::cin, path);

        // 去除首尾空白字符
        path.erase(0, path.find_first_not_of(" \t\n\r"));
        path.erase(path.find_last_not_of(" \t\n\r") + 1);

        if (path.empty()) {
            showError("输入错误", "文件路径不能为空");
            return "";
        }

        return path;
    }

    size_t promptFileSystemSize(size_t defaultSizeMB, const std::string& prompt) {
        std::string defaultPrompt = prompt.empty() ?
            ("请输入文件系统大小（单位：MB，默认" + std::to_string(defaultSizeMB) + "，输入0为默认值）：") :
            prompt;

        size_t input;
        std::cout << defaultPrompt;
        std::cin >> input;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        try {
            size_t sizeMB = input;
            if (sizeMB == 0) {
                return defaultSizeMB * BwtFS::MB;
            }
            return sizeMB * BwtFS::MB;
        } catch (const std::exception& e) {
            showWarning("输入格式错误，使用默认值：" + std::to_string(defaultSizeMB) + "MB");
            return defaultSizeMB * BwtFS::MB;
        }
    }

    void showSuccess(const std::string& operation, const std::string& detail) {
        LOG_INFO << operation << (detail.empty() ? "" : ": " + detail);
    }

    void showError(const std::string& operation, const std::string& detail) {
        LOG_ERROR << operation << (detail.empty() ? "" : ": " + detail);
    }

    void showWarning(const std::string& message) {
        LOG_WARNING << message;
    }

    void showInfo(const std::string& message) {
        LOG_INFO << message;
    }

    void showProgress(const std::string& message) {
        LOG_INFO << message;
    }

    bool confirm(const std::string& message) {
        std::cout << "\n" << message << " (y/N): ";
        std::string response;
        std::getline(std::cin, response);

        std::transform(response.begin(), response.end(), response.begin(), ::tolower);
        return response == "y" || response == "yes";
    }

    void pause() {
    #ifdef _WIN32
        system("pause");
    #else
        std::cout << "\n按回车键继续...";
        // std::cin.clear();
        // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    #endif
    }

    void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    }

    void showSeparator(int length, char character) {
        std::cout << std::string(length, character) << "\n";
    }

    bool isValidBwtFSPath(const std::string& path) {
        if (path.empty()) return false;

        // 检查文件扩展名
        size_t dotPos = path.find_last_of('.');
        if (dotPos == std::string::npos) {
            return false;
        }

        std::string extension = path.substr(dotPos);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        return extension == ".bwt";
    }

    std::string formatFileSize(size_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
        return oss.str();
    }

    void showFileSummary(const std::string& filePath, size_t fileSize) {
        std::cout << "\n文件信息摘要:\n";
        showSeparator(30);
        std::cout << "文件路径: " << filePath << "\n";
        std::cout << "文件大小: " << formatFileSize(fileSize) << "\n";

        if (fileSize > 0) {
            std::cout << "预计处理时间: ";
            if (fileSize < 1024 * 1024) {
                std::cout << "< 1秒\n";
            } else if (fileSize < 100 * 1024 * 1024) {
                std::cout << "1-10秒\n";
            } else if (fileSize < 1024 * 1024 * 1024) {
                std::cout << "10-60秒\n";
            } else {
                std::cout << "> 1分钟\n";
            }
        }
        showSeparator(30);
    }

    std::string promptToken(const std::string& prompt) {
        std::string token;
        std::cout << prompt;
        std::getline(std::cin, token);

        // 去除首尾空白字符
        token.erase(0, token.find_first_not_of(" \t\n\r"));
        token.erase(token.find_last_not_of(" \t\n\r") + 1);

        if (token.empty()) {
            showError("输入错误", "访问令牌不能为空");
            return "";
        }

        return token;
    }

    std::string promptOutputPath(const std::string& prompt) {
        std::string path;
        std::cout << prompt;
        std::getline(std::cin, path);

        // 去除首尾空白字符
        path.erase(0, path.find_first_not_of(" \t\n\r"));
        path.erase(path.find_last_not_of(" \t\n\r") + 1);

        // 空字符串表示输出到标准输出
        return path;
    }

    } // namespace UI
} // namespace BwtFS