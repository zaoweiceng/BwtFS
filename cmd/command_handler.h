#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>
#include "file_operations.h"

namespace BwtFS {
namespace Command {

/**
 * @brief 命令类型枚举
 */
enum class CommandType {
    INTERACTIVE,    // 交互模式
    CREATE_FS,      // 创建文件系统
    WRITE_FILE,     // 写入文件
    RETRIEVE_FILE,  // 获取文件
    DELETE_FILE,    // 删除文件
    SHOW_HELP,      // 显示帮助
    SHOW_VERSION,   // 显示版本
    SHOW_INFO,      // 显示信息
    UNKNOWN         // 未知命令
};

/**
 * @brief 命令参数结构
 */
struct CommandArgs {
    CommandType type = CommandType::UNKNOWN;
    std::vector<std::string> arguments;
    bool showHelp = false;
    bool showVersion = false;
    bool verbose = false;
};

/**
 * @brief 命令处理器类
 */
class CommandHandler {
public:
    /**
     * @brief 构造函数
     */
    CommandHandler();

    /**
     * @brief 析构函数
     */
    ~CommandHandler() = default;

    /**
     * @brief 解析命令行参数
     * @param argc 参数数量
     * @param argv 参数数组
     * @return 解析后的命令参数
     */
    CommandArgs parseArguments(int argc, char* argv[]);

    /**
     * @brief 执行命令
     * @param args 命令参数
     * @return 执行结果（0成功，非0失败）
     */
    int executeCommand(const CommandArgs& args);

    /**
     * @brief 运行交互模式
     * @return 执行结果（0成功，非0失败）
     */
    int runInteractiveMode();

    /**
     * @brief 运行文件系统创建模式
     * @return 执行结果（0成功，非0失败）
     */
    int runCreateFSMode();

    /**
     * @brief 运行文件写入模式
     * @param systemPath 文件系统路径
     * @param filePath 文件路径
     * @return 执行结果（0成功，非0失败）
     */
    int runWriteFileMode(const std::string& systemPath, const std::string& filePath);

    /**
     * @brief 运行文件获取模式
     * @param systemPath 文件系统路径
     * @param token 文件访问令牌
     * @param outputPath 输出文件路径（可选，默认输出到标准输出）
     * @return 执行结果（0成功，非0失败）
     */
    int runRetrieveFileMode(const std::string& systemPath, const std::string& token, const std::string& outputPath = "");

    /**
     * @brief 运行文件删除模式
     * @param systemPath 文件系统路径
     * @param token 文件访问令牌
     * @return 执行结果（0成功，非0失败）
     */
    int runDeleteFileMode(const std::string& systemPath, const std::string& token);

    /**
     * @brief 显示帮助信息
     */
    void showHelp();

    /**
     * @brief 显示版本信息
     */
    void showVersion();

    /**
     * @brief 设置详细模式
     * @param verbose 是否启用详细模式
     */
    void setVerbose(bool verbose);

private:
    bool m_verbose = false;

    /**
     * @brief 初始化BwtFS系统
     * @return 初始化是否成功
     */
    bool initializeBwtFS();

    /**
     * @brief 显示程序横幅
     */
    void showBanner();

    /**
     * @brief 验证命令参数
     * @param args 命令参数
     * @return 验证结果
     */
    bool validateArguments(const CommandArgs& args);

    /**
     * @brief 处理创建文件系统
     * @param systemPath 文件系统路径
     * @param sizeMB 文件系统大小（MB）
     * @return 执行结果
     */
    FileOps::OperationResult handleCreateFS(const std::string& systemPath, size_t sizeMB);

    /**
     * @brief 处理文件写入
     * @param systemPath 文件系统路径
     * @param filePath 文件路径
     * @return 执行结果
     */
    FileOps::OperationResult handleWriteFile(const std::string& systemPath, const std::string& filePath);

    /**
     * @brief 处理文件获取
     * @param systemPath 文件系统路径
     * @param token 文件访问令牌
     * @param outputPath 输出文件路径（可选）
     * @return 执行结果
     */
    FileOps::OperationResult handleRetrieveFile(const std::string& systemPath, const std::string& token, const std::string& outputPath = "");

    /**
     * @brief 处理文件删除
     * @param systemPath 文件系统路径
     * @param token 文件访问令牌
     * @return 执行结果
     */
    FileOps::OperationResult handleDeleteFile(const std::string& systemPath, const std::string& token);

    /**
     * @brief 处理错误情况
     * @param operation 操作名称
     * @param result 操作结果
     * @return 错误代码
     */
    int handleError(const std::string& operation, const FileOps::OperationResult& result);

    /**
     * @brief 显示BwtFS文件系统信息
     * @param systemPath 文件系统路径
     * @return 执行结果（0成功，非0失败）
     */
    int showBwtFSInfo(const std::string& systemPath);
};

} // namespace Command
} // namespace BwtFS

#endif // COMMAND_HANDLER_H