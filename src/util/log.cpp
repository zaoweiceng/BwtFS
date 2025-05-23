#include "util/log.h"
#include "util/ini_parser.h"
#include "util/date.h"
#include <iomanip>

void BwtFS::Util::Logger::init() {
    auto config = BwtFS::Config::getInstance();
    __level = BwtFS::Util::LogLevelFromString(config.get("logging", "log_level", "INFO"));
    __console = config.get("logging", "log_to_console", "true") == "true";
    __file = config.get("logging", "log_to_file", "false") == "true";
    __file_path = config.get("logging", "log_path", "");
    __file_path = __file_path.find_last_not_of('.') == std::string::npos 
                  ? __file_path + "-" + BwtFS::Util::timeToString(std::time(nullptr), "%Y%m%d") + ".log"
                  : __file_path.substr(0, __file_path.find_last_of('.')) + "-" +  BwtFS::Util::timeToString(std::time(nullptr), "%Y%m%d") + ".log";

}

BwtFS::Util::LogLevel  BwtFS::Util::LogLevelFromString(const std::string& levelStr) {
    if (levelStr == "DEBUG") return LogLevel::DEBUG;
    if (levelStr == "INFO") return LogLevel::INFO;
    if (levelStr == "WARNING") return LogLevel::WARNING;
    if (levelStr == "ERROR") return LogLevel::ERROR_;
    return LogLevel::INFO;
}

void BwtFS::Util::Logger::setLevel(LogLevel level){
    std::lock_guard<std::mutex> lock(__mutex);
    __level = level;
}

BwtFS::Util::LogLevel BwtFS::Util::Logger::getLevel() const{
    return __level;
}

void BwtFS::Util::Logger::setConsole(bool console){
    std::lock_guard<std::mutex> lock(__mutex);
    __console = console;
}

void BwtFS::Util::Logger::setFile(bool file, const std::string& file_path){
    std::lock_guard<std::mutex> lock(__mutex);
    __file = file;
    __file_path = file_path;
    if (__file && __file_path != "")
        __file_stream.open(__file_path, std::ios::out | std::ios::app);
}

std::string levelToString(BwtFS::Util::LogLevel level) {
    switch (level) {
        case BwtFS::Util::LogLevel::DEBUG:   return "DEBUG";
        case BwtFS::Util::LogLevel::INFO:    return "INFO";
        case BwtFS::Util::LogLevel::WARNING: return "WARNING";
        case BwtFS::Util::LogLevel::ERROR_:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string levelToStringConsole(BwtFS::Util::LogLevel level) {
    switch (level) {
        case BwtFS::Util::LogLevel::DEBUG:   return "\033[34mDEBUG\033[0m";   // Blue
        case BwtFS::Util::LogLevel::INFO:    return "\033[32mINFO\033[0m";    // Green
        case BwtFS::Util::LogLevel::WARNING: return "\033[33mWARNING\033[0m"; // Yellow
        case BwtFS::Util::LogLevel::ERROR_:   return "\033[31mERROR\033[0m";   // Red
        default:                             return "UNKNOWN";  // Reset
    }
}

std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    #ifdef _WIN32
    localtime_s(&tm, &time);
    #else
    localtime_r(&time, &tm);
    #endif
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string formatMessage(BwtFS::Util::LogLevel level, const std::string& message, const std::string& file, int line) {
    std::stringstream ss;
    ss << getCurrentTime() << " [" << levelToString(level) << "] "
       << file << ":" << line << " - " << message;
    return ss.str();
}

std::string formatMessageConsole(BwtFS::Util::LogLevel level, const std::string& message, const std::string& file, int line) {
    std::stringstream ss;
    ss << getCurrentTime() << " [" << levelToStringConsole(level) << "] "
       << file << ":" << line << " - " << message;
    return ss.str();
}

void BwtFS::Util::Logger::log(LogLevel level, const std::string& message, const std::string& file, int line) {
    std::lock_guard<std::mutex> lock(__mutex);
    if (__console) {
        std::cout << formatMessageConsole(level, message, file, line) << std::endl;
    }
    
    if (__file) {
        // 每天自动将日志保存到新的文件中
        if (std::time(nullptr) - __time > 60*60*24) {
            __file_stream.close();
            __time = std::time(nullptr);
        }
        if (!__file_stream.is_open()) {
            __file_path = __file_path.find_last_not_of('-') == std::string::npos 
                        ? __file_path + "-" + timeToString(std::time(nullptr), "%Y%m%d") + ".log"
                        : __file_path.substr(0, __file_path.find_last_of('-')) + "-" +  timeToString(std::time(nullptr), "%Y%m%d") + ".log";
            __file_stream.open(__file_path, std::ios::app);
        }
        if (__file_stream.good()) {
            __file_stream << formatMessage(level, message, file, line) << std::endl;
        }
    }
}