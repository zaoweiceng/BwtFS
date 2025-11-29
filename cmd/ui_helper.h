#ifndef UI_HELPER_H
#define UI_HELPER_H

#include <string>
#include <vector>
#include <iostream>

namespace BwtFS {
namespace UI {

/**
 * @brief 显示程序版本信息
 */
void showVersion();

/**
 * @brief 显示帮助信息
 */
void showHelp();

/**
 * @brief 显示程序横幅
 */
void showBanner();

/**
 * @brief 提示用户输入文件系统路径
 * @param prompt 提示信息
 * @return 用户输入的路径
 */
std::string promptFileSystemPath(const std::string& prompt = "请输入文件系统路径：");

/**
 * @brief 提示用户输入文件路径
 * @param prompt 提示信息
 * @return 用户输入的路径
 */
std::string promptFilePath(const std::string& prompt = "请输入要写入的文件路径：");

/**
 * @brief 提示用户输入文件系统大小
 * @param defaultSizeMB 默认大小（MB）
 * @param prompt 提示信息
 * @return 文件系统大小（字节）
 */
size_t promptFileSystemSize(size_t defaultSizeMB, const std::string& prompt = "");

/**
 * @brief 显示操作成功信息
 * @param operation 操作名称
 * @param detail 详细信息
 */
void showSuccess(const std::string& operation, const std::string& detail = "");

/**
 * @brief 显示操作失败信息
 * @param operation 操作名称
 * @param detail 详细信息
 */
void showError(const std::string& operation, const std::string& detail = "");

/**
 * @brief 显示警告信息
 * @param message 警告信息
 */
void showWarning(const std::string& message);

/**
 * @brief 显示信息提示
 * @param message 信息内容
 */
void showInfo(const std::string& message);

/**
 * @brief 显示进度信息
 * @param message 进度信息
 */
void showProgress(const std::string& message);

/**
 * @brief 确认用户操作
 * @param message 确认信息
 * @return 用户确认结果
 */
bool confirm(const std::string& message);

/**
 * @brief 等待用户按键继续
 */
void pause();

/**
 * @brief 清屏
 */
void clearScreen();

/**
 * @brief 显示分隔线
 * @param length 分隔线长度
 * @param character 分隔线字符
 */
void showSeparator(int length = 50, char character = '-');

/**
 * @brief 验证文件路径格式
 * @param path 文件路径
 * @return 是否为有效的BwtFS文件路径
 */
bool isValidBwtFSPath(const std::string& path);

/**
 * @brief 获取文件大小的可读格式
 * @param bytes 字节数
 * @return 格式化的大小字符串
 */
std::string formatFileSize(size_t bytes);

/**
 * @brief 显示文件信息摘要
 * @param filePath 文件路径
 * @param fileSize 文件大小
 */
void showFileSummary(const std::string& filePath, size_t fileSize);

} // namespace UI
} // namespace BwtFS

#endif // UI_HELPER_H