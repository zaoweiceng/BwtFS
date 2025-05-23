#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <ctime>
#include "config.h"
namespace BwtFS::Util{
    /*
    * 日志级别枚举
    * DEBUG: 调试信息
    * INFO: 一般信息
    * WARNING: 警告信息
    * ERROR: 错误信息
    * @author: zaoweiceng
    * @date: 2025-03-28
    */
    enum LogLevel{
        DEBUG,
        INFO,
        WARNING,
        ERROR_
    };
    LogLevel LogLevelFromString(const std::string& levelStr);
    /*
    * 日志类
    * 用于日志的输出
    * @author: zaoweiceng
    * @data: 2025-03-28
    */
    class Logger{
        public:
            // 获取单例对象
            // 全局均使用getInstance()获取Logger对象
            // 线程安全
            static Logger& getInstance(){
                static Logger instance;
                if (!instance.__init){
                    instance.init();
                }
                return instance;
            }
            // 设置日志级别
            // 传入日志级别
            void setLevel(LogLevel level);
            // 获取日志级别
            // 返回当前日志级别
            LogLevel getLevel() const;
            // 设置日志输出方式
            // 传入是否输出到控制台
            void setConsole(bool console);
            // 设置日志输出方式
            // 传入是否输出到文件
            // 传入文件路径
            void setFile(bool file, const std::string& file_path);
            // 记录日志
            // 传入日志级别，日志内容，文件名，行号
            void log(LogLevel level, const std::string& message, const std::string& file, int line);
        
        private:
            // 互斥锁，用于文件输出的线程安全
            std::mutex __mutex;
            // 日志级别
            LogLevel __level = [](const std::string& levelStr) {
                if (levelStr == "DEBUG") return LogLevel::DEBUG;
                if (levelStr == "INFO") return LogLevel::INFO;
                if (levelStr == "WARNING") return LogLevel::WARNING;
                if (levelStr == "ERROR") return LogLevel::ERROR_;
                return LogLevel::INFO;
            }(BwtFS::DefaultConfig::LOG_LEVEL);
            // 是否输出到控制台
            bool __console = BwtFS::DefaultConfig::LOG_TO_CONSOLE;
            
            // 是否输出到文件
            bool __file = BwtFS::DefaultConfig::LOG_TO_FILE;
            // 文件路径
            std::string __file_path = BwtFS::DefaultConfig::LOG_PATH;
            // 文件流
            std::ofstream __file_stream;
            // 初始化
            void init();
            // 是否初始化
            bool __init = false;
            // 时间记录
            unsigned long long __time = std::time(nullptr);
            // 构造函数
            // 私有化构造函数，禁止外部创建对象
            // 使用单例模式
            // 初始化日志级别为INFO
            // 初始化输出方式为控制台
            // 初始化文件路径为空
            Logger():__level(LogLevel::INFO), __console(true), __file(false), __file_path(""){
                if (__file && __file_path != "")
                    __file_stream.open(__file_path, std::ios::out | std::ios::app);
            }
            Logger(Logger const&) = delete;
            Logger& operator=(Logger const&) = delete;
            ~Logger(){}
    };
    /*
    * 日志流类
    * 用于日志的输出
    * 结合宏定义，简化日志类的使用
    * @author: zaoweiceng
    * @date: 2025-03-28
    */
    class LogStream {
        public:
            LogStream(LogLevel level, const std::string& file, int line)
                : level_(level), file_(file), line_(line) {}
            
            ~LogStream() {
                Logger::getInstance().log(level_, ss_.str(), file_, line_);
            }
            
            template<typename T>
            LogStream& operator<<(const T& msg) {
                ss_ << msg;
                return *this;
            }
            
        private:
            LogLevel level_;
            std::stringstream ss_;
            std::string file_;
            int line_;
    };
    // 日志宏定义
    #define LOG(level) \
            if (level < Logger::getInstance().getLevel()) {} \
            else BwtFS::Util::LogStream(level, __FILE__, __LINE__)
    // Debug日志
    #define LOG_DEBUG LOG(BwtFS::Util::LogLevel::DEBUG)
    // Info日志
    #define LOG_INFO LOG(BwtFS::Util::LogLevel::INFO)
    // Warning日志
    #define LOG_WARNING LOG(BwtFS::Util::LogLevel::WARNING)
    // Error日志
    #define LOG_ERROR LOG(BwtFS::Util::LogLevel::ERROR_)
    // Fatal日志
    #define LOG_FATAL LOG(BwtFS::Util::LogLevel::ERROR_) << "FATAL ERROR: " << __FILE__ << ":" << __LINE__ << " - "
    // 
    #define LOG_CIALLO std::cout << "\033[32mCiallo～(∠·ω< )⌒★\033[0m" << std::endl;
}


#endif