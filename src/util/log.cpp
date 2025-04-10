#include "util/log.h"
#include "util/ini_parser.h"
#include <iomanip>

void BwtFS::Util::Logger::init() {
    auto config = BwtFs::Config::getInstance();
    __level = BwtFS::Util::LogLevelFromString(config.get("logging", "log_level", "INFO"));
    __console = config.get("logging", "log_to_console", "true") == "true";
    __file = config.get("logging", "log_to_file", "false") == "true";
    __file_path = config.get("logging", "log_path", "");

}

BwtFS::Util::LogLevel  BwtFS::Util::LogLevelFromString(const std::string& levelStr) {
    if (levelStr == "DEBUG") return LogLevel::DEBUG;
    if (levelStr == "INFO") return LogLevel::INFO;
    if (levelStr == "WARNING") return LogLevel::WARNING;
    if (levelStr == "ERROR") return LogLevel::ERROR;
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
        case BwtFS::Util::LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

std::string levelToStringConsole(BwtFS::Util::LogLevel level) {
    switch (level) {
        case BwtFS::Util::LogLevel::DEBUG:   return "\033[34mDEBUG\033[0m";   // Blue
        case BwtFS::Util::LogLevel::INFO:    return "\033[32mINFO\033[0m";    // Green
        case BwtFS::Util::LogLevel::WARNING: return "\033[33mWARNING\033[0m"; // Yellow
        case BwtFS::Util::LogLevel::ERROR:   return "\033[31mERROR\033[0m";   // Red
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
        checkFileRollover();
        if (!__file_stream.is_open()) {
            __file_stream.open(__file_path, std::ios::app);
        }
        if (__file_stream.good()) {
            __file_stream << formatMessage(level, message, file, line) << std::endl;
        }
    }
}

void BwtFS::Util::Logger::checkFileRollover() {
    if (!__file) return;
    if (__file_path == "") return;
    if (!__file_stream.is_open()) {
        __file_stream.open(__file_path, std::ios::out | std::ios::app);
    }
    std::ifstream in(__file_path, std::ios::ate | std::ios::binary);
    if (in.good() && in.tellg() > 10 * 1024 * 1024) { // 10MB
        __file_stream.close();
        size_t lastDot = __file_path.find_last_of('$');
        std::string baseName = (lastDot == std::string::npos) ? __file_path : __file_path.substr(0, lastDot);
        std::string newName = baseName + "$" + getCurrentTime() + ".log";
        std::rename(__file_path.c_str(), newName.c_str());
        __file_stream.open(__file_path, std::ios::app);
    }
}