#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>
#include "file_operations.h"

namespace BwtFS {
    namespace Command {

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

    struct CommandArgs {
        CommandType type = CommandType::UNKNOWN;
        std::vector<std::string> arguments;
        bool showHelp = false;
        bool showVersion = false;
        bool verbose = false;
    };

    class CommandHandler {
    public:
        CommandHandler();

        ~CommandHandler() = default;

        CommandArgs parseArguments(int argc, char* argv[]);

        int executeCommand(const CommandArgs& args);

        int runInteractiveMode();

        int runCreateFSMode();

        int runWriteFileMode(const std::string& systemPath, const std::string& filePath);

        int runRetrieveFileMode(const std::string& systemPath, const std::string& token, const std::string& outputPath = "");

        int runDeleteFileMode(const std::string& systemPath, const std::string& token);

        void showHelp();

        void showVersion();

        void setVerbose(bool verbose);

    private:
        bool m_verbose = false;
        bool initializeBwtFS();
        void showBanner();
        bool validateArguments(const CommandArgs& args);
        FileOps::OperationResult handleCreateFS(const std::string& systemPath, size_t sizeMB);
        FileOps::OperationResult handleWriteFile(const std::string& systemPath, const std::string& filePath);
        FileOps::OperationResult handleRetrieveFile(const std::string& systemPath, const std::string& token, const std::string& outputPath = "");
        FileOps::OperationResult handleDeleteFile(const std::string& systemPath, const std::string& token);
        int handleError(const std::string& operation, const FileOps::OperationResult& result);
        int showBwtFSInfo(const std::string& systemPath);
    };

    } // namespace Command
} // namespace BwtFS

#endif // COMMAND_HANDLER_H