#include "command_handler.h"
#include "ui_helper.h"
#include "file_operations.h"
#include "BwtFS.h"
#include <algorithm>
#include <iomanip>
#include <functional>

namespace BwtFS {
namespace Command {

CommandHandler::CommandHandler() {
    // 构造函数实现
}

CommandArgs CommandHandler::parseArguments(int argc, char* argv[]) {
    CommandArgs args;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // 处理选项参数
        if (arg == "--help" || arg == "-h") {
            args.showHelp = true;
            args.type = CommandType::SHOW_HELP;
        } else if (arg == "--version" || arg == "-v") {
            args.showVersion = true;
            args.type = CommandType::SHOW_VERSION;
        } else if (arg == "--verbose") {
            args.verbose = true;
            m_verbose = true;
        } else if (arg == "--info") {
            args.type = CommandType::SHOW_INFO;
            // 下一个参数是文件系统路径
            if (i + 1 < argc) {
                args.arguments.push_back(argv[++i]);
            }
        } else if (arg == "create") {
            args.type = CommandType::CREATE_FS;
        } else {
            // 位置参数
            args.arguments.push_back(arg);
        }
    }

    // 根据参数数量确定命令类型（如果没有通过选项确定）
    if (args.type == CommandType::UNKNOWN) {
        if (args.arguments.empty()) {
            args.type = CommandType::INTERACTIVE;
        } else if (args.arguments.size() == 1) {
            // 可能是 create 命令或单文件路径
            if (args.arguments[0] == "create") {
                args.type = CommandType::CREATE_FS;
            } else {
                args.type = CommandType::INTERACTIVE;
            }
        } else if (args.arguments.size() == 2) {
            // Check if this is a retrieve command (file system path + token)
            std::string firstArg = args.arguments[0];
            std::string secondArg = args.arguments[1];

            if (FileOps::isValidToken(secondArg)) {
                args.type = CommandType::RETRIEVE_FILE;
            } else {
                args.type = CommandType::WRITE_FILE;
            }
        } else if (args.arguments.size() == 3) {
            // Could be retrieve with output file path: system_path token output_path
            if (FileOps::isValidToken(args.arguments[1])) {
                args.type = CommandType::RETRIEVE_FILE;
            }
        }
    }

    return args;
}

int CommandHandler::executeCommand(const CommandArgs& args) {
    try {
        // 显示横幅
        showBanner();

        // 处理特殊命令
        if (args.showHelp) {
            showHelp();
            return 0;
        }

        if (args.showVersion) {
            showVersion();
            return 0;
        }

        if (args.type == CommandType::SHOW_INFO && !args.arguments.empty()) {
            return showBwtFSInfo(args.arguments[0]);
        }

        // 初始化BwtFS系统
        if (!initializeBwtFS()) {
            UI::showError("系统初始化", "BwtFS系统初始化失败");
            return 1;
        }

        // 根据命令类型执行相应操作
        switch (args.type) {
            case CommandType::INTERACTIVE:
                return runInteractiveMode();

            case CommandType::CREATE_FS:
                return runCreateFSMode();

            case CommandType::WRITE_FILE:
                if (args.arguments.size() >= 2) {
                    return runWriteFileMode(args.arguments[0], args.arguments[1]);
                } else {
                    UI::showError("参数错误", "写入文件模式需要两个参数");
                    return 1;
                }

            case CommandType::RETRIEVE_FILE:
                if (args.arguments.size() >= 2) {
                    std::string outputPath = (args.arguments.size() >= 3) ? args.arguments[2] : "";
                    return runRetrieveFileMode(args.arguments[0], args.arguments[1], outputPath);
                } else {
                    UI::showError("参数错误", "文件获取模式需要至少两个参数");
                    return 1;
                }

            default:
                UI::showError("未知命令", "无法识别的命令类型");
                showHelp();
                return 1;
        }

    } catch (const std::exception& e) {
        UI::showError("执行异常", std::string("发生未预期的错误: ") + e.what());
        return 1;
    }
}

int CommandHandler::runInteractiveMode() {
    UI::showInfo("进入交互模式");

    while (true) {
        UI::showSeparator();
        std::cout << "请选择操作:\n";
        std::cout << "1. 写入文件到文件系统\n";
        std::cout << "2. 创建新的文件系统\n";
        std::cout << "3. 使用令牌获取文件\n";
        std::cout << "4. 显示文件系统信息\n";
        std::cout << "5. 退出程序\n";
        UI::showSeparator();

        std::cout << "请输入选项 (1-5): ";
        std::string choice;
        std::getline(std::cin, choice);

        // 去除空白字符
        choice.erase(0, choice.find_first_not_of(" \t\n\r"));
        choice.erase(choice.find_last_not_of(" \t\n\r") + 1);

        if (choice == "1") {
            // 写入文件
            std::string systemPath = UI::promptFileSystemPath();
            if (systemPath.empty()) continue;

            if (!FileOps::bwtFSExists(systemPath)) {
                UI::showError("文件系统错误", "指定的BwtFS文件系统不存在");
                continue;
            }

            std::string filePath = UI::promptFilePath();
            if (filePath.empty()) continue;

            runWriteFileMode(systemPath, filePath);

        } else if (choice == "2") {
            // 创建文件系统
            runCreateFSMode();

        } else if (choice == "3") {
            // 使用令牌获取文件
            std::string systemPath = UI::promptFileSystemPath();
            if (systemPath.empty()) continue;

            if (!FileOps::bwtFSExists(systemPath)) {
                UI::showError("文件系统错误", "指定的BwtFS文件系统不存在");
                continue;
            }

            std::string token = UI::promptToken();
            if (token.empty()) continue;

            std::string outputPath = UI::promptOutputPath();

            runRetrieveFileMode(systemPath, token, outputPath);

        } else if (choice == "4") {
            // 显示文件系统信息
            std::string systemPath = UI::promptFileSystemPath();
            if (!systemPath.empty()) {
                showBwtFSInfo(systemPath);
            }

        } else if (choice == "5") {
            // 退出
            UI::showInfo("感谢使用BwtFS文件系统");
            break;

        } else {
            UI::showWarning("无效选项，请重新选择");
        }
    }

    return 0;
}

int CommandHandler::runCreateFSMode() {
    UI::showProgress("创建文件系统模式");

    // 获取文件系统路径
    std::string systemPath = UI::promptFileSystemPath();
    if (systemPath.empty()) {
        return handleError("创建文件系统", FileOps::OperationResult(false, "文件系统路径不能为空"));
    }

    // 检查文件是否已存在
    if (FileOps::bwtFSExists(systemPath)) {
        if (!UI::confirm("文件系统已存在，是否覆盖？")) {
            UI::showInfo("操作已取消");
            return 0;
        }
    }

    // 获取文件系统大小
    auto config = BwtFS::Config::getInstance();
    size_t defaultSizeMB = std::stoull(config["system"]["size"]) / BwtFS::MB;
    size_t fileSize = UI::promptFileSystemSize(defaultSizeMB);

    if (fileSize * BwtFS::MB < BwtFS::DefaultConfig::SYSTEM_FILE_MIN_SIZE) {
        UI::showWarning("文件系统大小小于最小要求，将使用最小值: " +
                        UI::formatFileSize(BwtFS::DefaultConfig::SYSTEM_FILE_MIN_SIZE));
        fileSize = BwtFS::DefaultConfig::SYSTEM_FILE_MIN_SIZE;
    }

    // 创建文件系统
    size_t sizeMB = fileSize / BwtFS::MB;
    UI::showInfo("正在创建文件系统: " + systemPath + "，大小: " + std::to_string(sizeMB) + "MB");

    auto result = handleCreateFS(systemPath, sizeMB);
    if (result.success) {
        UI::showSuccess("文件系统创建成功", systemPath);
        UI::showFileSummary(systemPath, fileSize);
    } else {
        return handleError("创建文件系统", result);
    }

    return 0;
}

int CommandHandler::runWriteFileMode(const std::string& systemPath, const std::string& filePath) {
    // 确定哪个是文件系统路径，哪个是文件路径
    std::string fsPath, fPath;

    if (UI::isValidBwtFSPath(systemPath)) {
        fsPath = systemPath;
        fPath = filePath;
    } else if (UI::isValidBwtFSPath(filePath)) {
        fsPath = filePath;
        fPath = systemPath;
    } else {
        UI::showError("参数错误", "无法确定BwtFS文件系统路径");
        return 1;
    }

    // 验证文件系统
    if (!FileOps::bwtFSExists(fsPath)) {
        UI::showError("文件系统错误", "BwtFS文件系统不存在: " + fsPath);
        return 1;
    }

    // 验证文件
    if (!FileOps::fileExists(fPath)) {
        UI::showError("文件错误", "文件不存在: " + fPath);
        return 1;
    }

    size_t fileSize = FileOps::getFileSize(fPath);
    if (fileSize == 0) {
        UI::showError("文件错误", "文件为空: " + fPath);
        return 1;
    }

    UI::showFileSummary(fPath, fileSize);

    // 写入文件
    auto result = handleWriteFile(fsPath, fPath);
    if (result.success) {
        UI::showSuccess("文件写入成功", "访问令牌: " + result.token);
        UI::showInfo("请妥善保存访问令牌，用于后续文件访问");
    } else {
        return handleError("写入文件", result);
    }

    return 0;
}

int CommandHandler::runRetrieveFileMode(const std::string& systemPath, const std::string& token, const std::string& outputPath) {
    // 确定文件系统路径
    std::string fsPath;

    if (UI::isValidBwtFSPath(systemPath)) {
        fsPath = systemPath;
    } else {
        UI::showError("参数错误", "无效的BwtFS文件系统路径: " + systemPath);
        return 1;
    }

    // 验证文件系统
    if (!FileOps::bwtFSExists(fsPath)) {
        UI::showError("文件系统错误", "BwtFS文件系统不存在: " + fsPath);
        return 1;
    }

    // 验证token
    if (!FileOps::isValidToken(token)) {
        UI::showError("Token错误", "无效的访问令牌格式");
        return 1;
    }

    UI::showInfo("正在从文件系统获取文件: " + fsPath);
    UI::showInfo("使用访问令牌: " + token.substr(0, 10) + "...");

    // 获取文件
    auto result = handleRetrieveFile(fsPath, token, outputPath);
    if (result.success) {
        if (!outputPath.empty()) {
            UI::showSuccess("文件获取成功", "输出文件: " + outputPath);
            UI::showInfo("文件大小: " + UI::formatFileSize(result.bytesProcessed));
        } else {
            UI::showSuccess("文件获取成功", "文件已输出到标准输出");
            UI::showInfo("文件大小: " + UI::formatFileSize(result.bytesProcessed));
        }
    } else {
        return handleError("获取文件", result);
    }

    return 0;
}

void CommandHandler::showHelp() {
    UI::showHelp();
}

void CommandHandler::showVersion() {
    UI::showVersion();
}

void CommandHandler::setVerbose(bool verbose) {
    m_verbose = verbose;
}

bool CommandHandler::initializeBwtFS() {
    try {
        init();
        auto config = BwtFS::Config::getInstance();
        if (m_verbose) {
            LOG_INFO << "BwtFS initialized successfully.";
            LOG_DEBUG << "Verbose mode enabled.";
        }
        return true;
    } catch (const std::exception& e) {
        if (m_verbose) {
            LOG_ERROR << "BwtFS initialization failed: " << e.what();
        }
        return false;
    }
}

void CommandHandler::showBanner() {
    UI::showBanner();
}

bool CommandHandler::validateArguments(const CommandArgs& args) {
    // 基本验证逻辑
    switch (args.type) {
        case CommandType::WRITE_FILE:
            return args.arguments.size() >= 2;

        case CommandType::SHOW_INFO:
            return !args.arguments.empty();

        case CommandType::CREATE_FS:
        case CommandType::INTERACTIVE:
            return true;

        default:
            return false;
    }
}

FileOps::OperationResult CommandHandler::handleCreateFS(const std::string& systemPath, size_t sizeMB) {
    return FileOps::createBwtFS(systemPath, sizeMB);
}

FileOps::OperationResult CommandHandler::handleWriteFile(const std::string& systemPath, const std::string& filePath) {
    // 创建进度回调函数
    auto progressCallback = [this](size_t bytesWritten, size_t totalBytes) {
        if (m_verbose) {
            double progress = static_cast<double>(bytesWritten) / totalBytes * 100.0;
            std::cout << "\r进度: " << std::fixed << std::setprecision(1) << progress << "% "
                      << "(" << UI::formatFileSize(bytesWritten) << "/" << UI::formatFileSize(totalBytes) << ")";
            std::cout.flush();
        }
    };

    auto result = FileOps::writeFileToBwtFSWithProgress(systemPath, filePath, progressCallback);

    if (m_verbose) {
        std::cout << "\n"; // 进度显示换行
    }

    return result;
}

FileOps::OperationResult CommandHandler::handleRetrieveFile(const std::string& systemPath, const std::string& token, const std::string& outputPath) {
    // 创建进度回调函数
    auto progressCallback = [this](size_t bytesWritten, size_t totalBytes) {
        if (m_verbose && totalBytes > 0) {
            double progress = static_cast<double>(bytesWritten) / totalBytes * 100.0;
            std::cout << "\r进度: " << std::fixed << std::setprecision(1) << progress << "% "
                      << "(" << UI::formatFileSize(bytesWritten) << "/" << UI::formatFileSize(totalBytes) << ")";
            std::cout.flush();
        }
    };

    // 根据是否有输出路径选择相应的函数
    FileOps::OperationResult result;
    if (!outputPath.empty()) {
        result = FileOps::retrieveFileFromBwtFS(systemPath, token, outputPath);
    } else {
        result = FileOps::retrieveFileFromBwtFS(systemPath, token);
    }

    if (m_verbose) {
        std::cout << "\n"; // 进度显示换行
    }

    return result;
}

int CommandHandler::handleError(const std::string& operation, const FileOps::OperationResult& result) {
    UI::showError(operation, result.message);
    return 1;
}

int CommandHandler::showBwtFSInfo(const std::string& systemPath) {
    if (!FileOps::bwtFSExists(systemPath)) {
        UI::showError("文件系统错误", "BwtFS文件系统不存在: " + systemPath);
        return 1;
    }

    UI::showProgress("正在获取文件系统信息");
    auto info = FileOps::getBwtFSInfo(systemPath);

    if (!info.isValid) {
        UI::showError("获取信息失败", "无法读取文件系统信息");
        return 1;
    }

    UI::showSuccess("文件系统信息获取成功");
    UI::showSeparator(50);
    std::cout << "文件系统路径: " << systemPath << "\n";
    std::cout << "总大小: " << UI::formatFileSize(info.totalSize) << "\n";
    std::cout << "已使用: " << UI::formatFileSize(info.usedSize) << "\n";
    std::cout << "可用空间: " << UI::formatFileSize(info.freeSize) << "\n";
    std::cout << "块大小: " << UI::formatFileSize(info.blockSize) << "\n";
    std::cout << "创建时间: " << info.createTime << "\n";
    std::cout << "修改时间: " << info.modifyTime << "\n";

    if (info.totalSize > 0) {
        double usagePercent = static_cast<double>(info.usedSize) / info.totalSize * 100.0;
        std::cout << "使用率: " << std::fixed << std::setprecision(1) << usagePercent << "%\n";
    }

    UI::showSeparator(50);

    return 0;
}

} // namespace Command
} // namespace BwtFS