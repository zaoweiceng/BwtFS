#include "command_handler.h"
#include "ui_helper.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        // 创建命令处理器
        BwtFS::Command::CommandHandler handler;

        // 解析命令行参数
        auto args = handler.parseArguments(argc, argv);

        // 执行命令
        int result = handler.executeCommand(args);

        // 如果需要等待用户输入（非错误退出且非帮助/版本显示）
        if (result == 0 &&
            args.type != BwtFS::Command::CommandType::SHOW_HELP &&
            args.type != BwtFS::Command::CommandType::SHOW_VERSION &&
            args.type != BwtFS::Command::CommandType::SHOW_INFO) {

            BwtFS::UI::pause();
        }

        return result;

    } catch (const std::exception& e) {
        std::cerr << "\n✗ 程序执行异常: " << e.what() << std::endl;

        // 等待用户按键以便查看错误信息
        BwtFS::UI::pause();

        return 1;

    } catch (...) {
        std::cerr << "\n✗ 程序执行发生未知异常" << std::endl;

        // 等待用户按键以便查看错误信息
        BwtFS::UI::pause();

        return 1;
    }
}