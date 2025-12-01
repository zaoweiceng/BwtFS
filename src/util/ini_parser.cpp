#include "util/ini_parser.h"
#include "util/log.h"

using BwtFS::Util::Logger;
namespace fs = std::filesystem;

void trim(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
}

void BwtFS::Config::init() {
    // LOG_INFO << "Config path: " << config_path;
    config_path = fs::path(BwtFS::DefaultConfig::CONFIG_PATH).make_preferred().string();
    
    if (config_path.empty()) {
        LOG_WARNING << "Config path is empty.";
        LOG_WARNING << "Using default config and generate config file.";
        config_path = BwtFS::DefaultConfig::CONFIG_PATH;
    }
    if (config_path.find(".ini") == std::string::npos) {
        LOG_WARNING << "Config file is not .ini file.";
        LOG_WARNING << "Using default config and generate config file.";
        config_path += ".ini";
    }
    load();
}

bool BwtFS::Config::load() {
    // 如果配置文件不存在，则创建默认配置
    // LOG_INFO << "Loading config file: " << config_path;
    if (!fs::exists(config_path) || config_path.empty()) {
        LOG_WARNING << "Config file does not exist.";
        LOG_WARNING << "Using default config file.";
        return false;
    }

    // 读取配置文件
    std::ifstream file(config_path.c_str());
    if (!file.is_open()) {
        LOG_ERROR << "Failed to open config file: " << config_path;
        return false;
    }

    std::string currentSection;
    std::string line;

    while (std::getline(file, line)) {
        trim(line);

        // 跳过空行和注释
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // 处理节(section)
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        // 处理键值对
        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            trim(key);
            trim(value);

            if (!currentSection.empty() && !key.empty()) {
                config_data[currentSection][key] = value;
            }
        }
    }

    file.close();
    return true;
}

bool BwtFS::Config::save() const {
    std::ofstream file(config_path.c_str());
    if (!file.is_open()) {
        LOG_ERROR << "Failed to open config file for writing: " << config_path;
        return false;
    }

    for (const auto& [section, kvPairs] : config_data) {
        file << "[" << section << "]\n";
        for (const auto& [key, value] : kvPairs) {
            file << key << "=" << value << "\n";
        }
        file << "\n";
    }

    file.close();
    LOG_INFO << "Config file saved: " << config_path;
    return true;
}

std::string BwtFS::Config::get(const std::string& section, const std::string& key, const std::string& default_value) const {
    auto sectionIt = config_data.find(section);
    if (sectionIt != config_data.end()) {
        auto keyIt = sectionIt->second.find(key);
        if (keyIt != sectionIt->second.end()) {
            return keyIt->second;
        }
    }
    return default_value;
}

void BwtFS::Config::set(const std::string& section, const std::string& key, const std::string& value) {
    config_data[section][key] = value;
    save();
    LOG_INFO << "Config updated: [" << section << "] " << key << "=" << value;
}

bool BwtFS::Config::createDefaultConfig() {
    // 确保目录存在
    auto parentPath = fs::path(config_path).parent_path();
    if (!parentPath.empty()) {
        std::error_code ec;
        fs::create_directories(parentPath, ec);
        if (ec) {
            LOG_ERROR << "Failed to create config directories: " << ec.message();
            return false;
        }
    }

    // 将默认配置写入文件
    std::ofstream file(config_path.c_str());
    if (!file.is_open()) {
        LOG_ERROR << "Failed to open config file for writing: " << config_path;
        return false;
    }

    file << "# This is an auto-generated configuration file\n\n";

    for (const auto& [section, kvPairs] : config_data) {
        file << "[" << section << "]\n";
        for (const auto& [key, value] : kvPairs) {
            file << key << "=" << value << "\n";
        }
        file << "\n";
    }

    file.close();
    LOG_INFO << "Default config file created: " << config_path;
    return true;
}

