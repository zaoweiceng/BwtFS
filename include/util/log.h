#ifndef LOG_H
#define LOG_H
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <ctime>
namespace BwtFS::Util{
    enum LogLevel{
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };
    class Logger{
        /*
        * 日志类
        * 用于日志的输出
        * @author: zaoweiceng
        * @data: 2025-03-28
        */
        public:
            static Logger& getInstance(){
                static Logger instance;
                return instance;
            }

            void setLevel(LogLevel level);
            LogLevel getLevel() const;
            void setConsole(bool console);
            void setFile(bool file, const std::string& file_path);
            void log(LogLevel level, const std::string& message, const std::string& file, int line);
            void checkFileRollover();
        
        private:
            std::mutex __mutex;
            LogLevel __level;
            bool __console;
            bool __file;
            std::string __file_path;
            std::ofstream __file_stream;
            Logger():__level(LogLevel::INFO), __console(true), __file(false), __file_path(""){
                if (__file && __file_path != "")
                    __file_stream.open(__file_path, std::ios::out | std::ios::app);
            }
            Logger(Logger const&) = delete;
            Logger& operator=(Logger const&) = delete;
            ~Logger(){}
    };
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
    #define LOG(level) \
            if (level < Logger::getInstance().getLevel()) {} \
            else BwtFS::Util::LogStream(level, __FILE__, __LINE__)

    #define LOG_DEBUG LOG(BwtFS::Util::LogLevel::DEBUG)
    #define LOG_INFO LOG(BwtFS::Util::LogLevel::INFO)
    #define LOG_WARNING LOG(BwtFS::Util::LogLevel::WARNING)
    #define LOG_ERROR LOG(BwtFS::Util::LogLevel::ERROR)
    #define LOG_FATAL LOG(BwtFS::Util::LogLevel::ERROR) << "FATAL ERROR: " << __FILE__ << ":" << __LINE__ << " - "
}


#endif