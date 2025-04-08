#ifndef INI_PARSER_H
#define INI_PARSER_H
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <system_error>
#include <utility>
#include "config.h"
#include <memory>
namespace BwtFs{
    class Config{
        public:
            static Config& getInstance(){
                static Config instance;
                static bool initialized = false;
                if (!initialized) {
                    instance.init(); // 仅在第一次调用时初始化
                    initialized = true;
                }
                return instance;
            }
            // 加载配置文件，如果不存在则创建默认配置
            bool load();

            // 获取配置值
            std::string get(const std::string& section, const std::string& key, const std::string& default_value = "") const;

            // 设置配置值
            void set(const std::string& section, const std::string& key, const std::string& value);

            // 保存配置文件
            bool save() const;

            // 初始化
            void init();

            // 重载[]运算符
            std::unordered_map<std::string, std::string> operator[](const std::string& key) const {
                return config_data.at(key);
            }

        private:
            std::string config_path;
            std::unordered_map<std::string, std::unordered_map<std::string, std::string>> config_data = {
                {"logging", {
                    {"log_level", BwtFS::DefaultConfig::LOG_LEVEL}, 
                    {"log_path", BwtFS::DefaultConfig::LOG_PATH}, 
                    {"log_to_file", BwtFS::DefaultConfig::LOG_TO_FILE ? "true" : "false"}, 
                    {"log_to_console", BwtFS::DefaultConfig::LOG_TO_CONSOLE ? "true" : "false"}
                }},
                {"system_file", {
                    {"path", BwtFS::DefaultConfig::SYSTEM_FILE_PATH}, 
                    {"size", std::to_string(BwtFS::DefaultConfig::SYSTEM_FILE_SIZE)}, 
                    {"prefix", BwtFS::DefaultConfig::SYSTEM_FILE_PREFIX}
                }}
            };

            // 创建默认配置文件
            bool createDefaultConfig();
    };

}
#endif