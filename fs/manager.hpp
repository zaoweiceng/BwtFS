#include <filesystem>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "BwtFS.h"
#include "json.hpp"

using BwtFS::Util::Logger;
using json = nlohmann::json;

struct SystemInfo{
    size_t file_size;
    size_t block_size;
    size_t block_count;
    size_t used_size;
    size_t total_size;
    size_t free_size;
    unsigned long long create_time;
    unsigned long long modify_time;
};

class SystemManager{
    private:
        bool fileExists(const std::string& file_path){
            return std::filesystem::exists(file_path);
        }
        std::shared_ptr<BwtFS::System::FileSystem> filesystem_;

    public:
        SystemManager(std::string system_path){
            if(!fileExists(system_path)){
                LOG_ERROR << "BwtFS system file does not exist: " << system_path;
                throw std::runtime_error("BwtFS system file does not exist: " + system_path);
            }
            filesystem_ = BwtFS::System::openBwtFS(system_path);

        }
        SystemManager(){}
        void init(std::string system_path){
            if(!fileExists(system_path)){
                LOG_ERROR << "BwtFS system file does not exist: " << system_path;
                throw std::runtime_error("BwtFS system file does not exist: " + system_path);
            }
            filesystem_ = BwtFS::System::openBwtFS(system_path);
        }
        ~SystemManager(){
        }
        // getSystemInfo
        SystemInfo getSystemInfo(){
            SystemInfo info;
            // LOG_DEBUG << "Getting system info from filesystem.";
            info.file_size = filesystem_->getFileSize();
            info.block_size = filesystem_->getBlockSize();
            info.block_count = filesystem_->getBlockCount();
            info.used_size = filesystem_->getFilesSize() - filesystem_->getFreeSize();
            info.total_size = filesystem_->getFileSize();
            info.free_size = filesystem_->getFreeSize();
            info.create_time = filesystem_->getCreateTime();
            info.modify_time = filesystem_->getModifyTime();
            // LOG_DEBUG << "System info retrieved: "
            //           << "file_size=" << info.file_size << ", "
            //           << "block_size=" << info.block_size << ", "
            //           << "block_count=" << info.block_count << ", "
            //           << "used_size=" << info.used_size << ", "
            //           << "total_size=" << info.total_size << ", "
            //           << "free_size=" << info.free_size << ", "
            //           << "create_time=" << info.create_time << ", "
            //           << "modify_time=" << info.modify_time;
            return info;
        }
};

// 文件节点信息结构体
struct FileNode {
    std::string name;
    bool is_dir;
    size_t file_size;    // 仅文件有值
    std::string token;  // 仅文件有值
    json* node_ptr;     // 指向原始JSON节点的指针（用于惰性解析）
    
    FileNode(const std::string& n = "", bool d = false, const std::string& t = "", size_t file_size = 0, json* ptr = nullptr)
        : name(n), is_dir(d), file_size(file_size), token(t), node_ptr(ptr) {}
};

// 文件目录管理类
class FileManager {
private:
    json root_json;               // 完整的JSON结构
    std::string file_path;      // JSON文件路径（用于保存）
    
    // 分割路径为组件
    std::vector<std::string> splitPath(const std::string& path) {
        std::vector<std::string> components;
        std::stringstream ss(path);
        std::string component;
        
        // 分割路径，跳过空组件
        while (std::getline(ss, component, '/')) {
            if (!component.empty()) {
                components.push_back(component);
            }
        }
        
        return components;
    }
    
    // 获取指定路径的JSON节点（惰性解析）
    json* getNodeAtPath(const std::string& path, bool create_parents = false) {
        std::vector<std::string> components = splitPath(path);

        LOG_DEBUG << "getNodeAtPath path: '" << path << "' components: " << components.size();

        // 空路径或根路径
        if (components.empty()) {
            LOG_DEBUG << "getNodeAtPath returning root for empty path";
            return &root_json;
        }

        json* current = &root_json;

        for (size_t i = 0; i < components.size(); i++) {
            const std::string& component = components[i];
            LOG_DEBUG << "getNodeAtPath processing component: '" << component << "' (index " << i << ")";

            // 如果节点不存在
            if (!current->contains(component)) {
                LOG_DEBUG << "Node does not exist: " << component;
                if (create_parents && i == components.size() - 1) {
                    // 创建最后一级节点
                    (*current)[component] = json::object();
                } else if (create_parents) {
                    // 创建中间目录
                    json new_dir;
                    new_dir["is_dir"] = true;
                    new_dir["children"] = json::object();
                    (*current)[component] = new_dir;
                    current = &(*current)[component]["children"];
                    LOG_DEBUG << "Created parent directory and moved to children";
                } else {
                    LOG_DEBUG << "Node not found and not creating parents";
                    return nullptr;  // 节点不存在
                }
            } else {
                // 节点存在，解析其is_dir
                json& node = (*current)[component];
                LOG_DEBUG << "Found existing node: " << component << " is_dir: " << (node.contains("is_dir") ? node["is_dir"].get<bool>() : false);

                if (!node.contains("is_dir")) {
                    LOG_ERROR << "Invalid node structure (no is_dir): " << component;
                    return nullptr;  // 无效节点结构
                }

                bool is_dir = node["is_dir"];

                if (is_dir) {
                    // 如果是目录且不是最后一个组件，进入children
                    if (i < components.size() - 1) {
                        if (!node.contains("children")) {
                            LOG_ERROR << "Directory has no children field: " << component;
                            return nullptr;
                        }
                        current = &node["children"];
                        LOG_DEBUG << "Moved to children of: " << component;
                    } else {
                        // 最后一个组件，返回目录节点本身
                        current = &node;
                        // LOG_DEBUG << "Last component is directory, returning node itself";
                    }
                } else {
                    // 如果是文件，只能作为最后一个组件
                    if (i < components.size() - 1) {
                        // LOG_ERROR << "Attempting to access child of file: " << component;
                        return nullptr;  // 试图访问文件的子节点
                    }
                    current = &node;
                    // LOG_DEBUG << "Last component is file, returning file node";
                }
            }
        }

        LOG_DEBUG << "getNodeAtPath returning node for path: " << path;
        return current; // 返回最终节点
    }
    
    // 获取父节点和子节点名
    std::pair<json*, std::string> getParentAndName(const std::string& path) {
        std::vector<std::string> components = splitPath(path);
        LOG_DEBUG << "Getting parent and name for path: " << path << ", components size: " << components.size();
        if (components.empty()) {
            return {&root_json, ""};  // 根路径
        }

        std::string name = components.back();
        components.pop_back();

        // 重建父路径
        std::string parent_path = components.empty() ? "" : "/";
        for (const auto& comp : components) {
            parent_path += comp + "/";
        }
        if (!parent_path.empty() && parent_path.back() == '/') {
            parent_path = parent_path.substr(0, parent_path.length() - 1);
        }

        LOG_DEBUG << "Parent path: '" << parent_path << "', name: '" << name << "'";

        // 如果是根目录下的文件，直接返回根节点
        json* parent = parent_path.empty() ? &root_json : getNodeAtPath(parent_path);
        return {parent, name};
    }
    
    // 递归删除目录
    void removeDirectory(json& dir_node) {
        if (dir_node.contains("children") && dir_node["children"].is_object()) {
            for (auto it = dir_node["children"].begin(); it != dir_node["children"].end(); ++it) {
                if (it.value().contains("is_dir") && it.value()["is_dir"]) {
                    removeDirectory(it.value());
                }
            }
        }
    }

public:
    FileManager() {
        root_json = json::object();
    }
    FileManager(const std::string& initial_path) {
        loadFromFile(initial_path);
        file_path = initial_path;
    }
    ~FileManager() {}
    
    // 从文件加载JSON
    bool loadFromFile(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                LOG_ERROR << "无法打开文件: " << filename;
                return false;
            }
            
            file >> root_json;
            LOG_INFO << "成功加载JSON文件: " << filename;
            file.close();
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR << "加载JSON文件失败: " << e.what();
            return false;
        }
    }
    
    // 保存到文件
    bool saveToFile(const std::string& filename) {
        try {
            // 确保目录存在
            std::filesystem::path filepath(filename);
            std::filesystem::path dirpath = filepath.parent_path();
            if (!dirpath.empty() && !std::filesystem::exists(dirpath)) {
                std::filesystem::create_directories(dirpath);
                LOG_DEBUG << "Created directory: " << dirpath.string();
            }

            // 确保文件是可写的
            std::ofstream file(filename, std::ios::trunc | std::ios::out);
            if (!file.is_open()) {
                LOG_ERROR << "无法打开文件进行写入: " << filename;
                return false;
            }

            // 强制刷新到磁盘
            std::string json_content = root_json.dump(4);
            file << json_content << std::flush;
            file.close();

            LOG_DEBUG << "JSON content written: " << json_content;

            // 验证文件是否被正确写入
            std::ifstream verify_file(filename);
            if (!verify_file.is_open()) {
                LOG_ERROR << "无法验证写入的文件: " << filename;
                return false;
            }

            std::string content((std::istreambuf_iterator<char>(verify_file)),
                               std::istreambuf_iterator<char>());
            verify_file.close();

            if (content.empty()) {
                LOG_ERROR << "JSON文件写入后为空: " << filename;
                return false;
            }

            LOG_DEBUG << "成功保存JSON文件: " << filename << " (大小: " << content.size() << " 字节)";
            LOG_DEBUG << "文件内容: " << content;
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR << "保存JSON文件失败: " << e.what();
            return false;
        }
    }
    
    // 列出目录内容（惰性解析）
    std::vector<FileNode> listDir(const std::string& path) {
        std::vector<FileNode> result;
        LOG_DEBUG << "listDir called for path: '" << path << "'";

        // 特殊处理根目录
        if (path == "/" || path.empty()) {
            LOG_DEBUG << "Handling root directory listing";
            // 根目录直接遍历root_json的顶层元素
            for (auto it = root_json.begin(); it != root_json.end(); ++it) {
                const std::string& name = it.key();
                if (it.value().contains("is_dir")) {
                    bool is_dir = it.value()["is_dir"];
                    std::string token = is_dir ? "" : (it.value().contains("token") ? it.value()["token"] : "");
                    size_t file_size = is_dir ? 0 : (it.value().contains("file_size") ? it.value()["file_size"].get<size_t>() : 0);
                    LOG_DEBUG << "Found root item: " << name << " (dir: " << is_dir << ")";
                    result.emplace_back(name, is_dir, token, file_size, &it.value());
                }
            }
            return result;
        }

        // 非根目录的处理
        json* node = getNodeAtPath(path);
        if (!node) {
            LOG_ERROR << "Node not found for path: " << path;
            return result;
        }

        if (!node->contains("is_dir") || !(*node)["is_dir"]) {
            LOG_ERROR << "Path is not a directory: " << path << " node: " << node->dump();
            return result;
        }

        // 检查是否有children字段
        if (!node->contains("children") || !(*node)["children"].is_object()) {
            LOG_ERROR << "Directory structure invalid for path: " << path;
            return result;
        }

        // 遍历children，只解析当前层级
        for (auto it = (*node)["children"].begin(); it != (*node)["children"].end(); ++it) {
            const std::string& name = it.key();
            if (it.value().contains("is_dir")) {
                bool is_dir = it.value()["is_dir"];
                std::string token = is_dir ? "" : (it.value().contains("token") ? it.value()["token"] : "");
                size_t file_size = is_dir ? 0 : (it.value().contains("file_size") ? it.value()["file_size"].get<size_t>() : 0);
                LOG_DEBUG << "Found child item: " << name << " (dir: " << is_dir << ")";
                result.emplace_back(name, is_dir, token, file_size, &it.value());
            }
        }

        return result;
    }
    
    // 获取文件信息
    FileNode getFile(const std::string& path) {
        LOG_DEBUG << "getFile called for path: '" << path << "'";

        // 特殊处理根路径
        if (path == "/" || path.empty()) {
            return FileNode("", true, "", 0, nullptr); // 根目录
        }

        auto [parent, name] = getParentAndName(path);
        LOG_DEBUG << "getFile - parent: " << (parent ? "valid" : "null") << ", name: '" << name << "'";

        if (!parent || name.empty()) {
            LOG_ERROR << "文件不存在: " << path;
            return FileNode();
        }

        // 检查父节点是否为根目录
        json* node_ptr;
        if (parent == &root_json) {
            // 直接在根目录下查找
            if (!parent->contains(name)) {
                // 对于系统临时文件（如Finder创建的），静默处理而不报错
                if (name.find("._") == 0 || name == ".DS_Store" || name.empty()) {
                    LOG_DEBUG << "系统文件不存在，返回空节点: " << path;
                } else {
                    LOG_DEBUG << "文件不存在: " << path;
                }
                return FileNode();
            }
            node_ptr = &(*parent)[name];
        } else {
            // 在children中查找
            if (!parent->contains("children") || !(*parent)["children"].contains(name)) {
                // 对于系统临时文件（如Finder创建的），静默处理而不报错
                if (name.find("._") == 0 || name == ".DS_Store" || name.empty()) {
                    LOG_DEBUG << "系统文件不存在，返回空节点: " << path;
                } else {
                    LOG_DEBUG << "文件不存在: " << path;
                }
                return FileNode();
            }
            node_ptr = &(*parent)["children"][name];
        }

        json& node = *node_ptr;
        if (!node.contains("is_dir")) {
            LOG_ERROR << "无效的节点结构: " << path;
            return FileNode();
        }

        bool is_dir = node["is_dir"];
        if (is_dir) {
            LOG_DEBUG << "路径是目录: " << path;
            return FileNode(name, true, "", 0, &node);
        }

        std::string token = node.contains("token") ? node["token"] : "";
        size_t file_size = node.contains("file_size") ? node["file_size"].get<size_t>() : 0;
        LOG_DEBUG << "Found file: " << path << " token: '" << token << "' size: " << file_size;
        return FileNode(name, false, token, file_size, &node);
    }
    
    // 创建目录
    bool createDir(const std::string& path) {
        // 确保路径格式正确，去掉前导斜杠的存储
        std::vector<std::string> components = splitPath(path);

        // 根目录特殊处理
        if (components.empty()) {
            return true; // 根目录已存在
        }

        LOG_DEBUG << "createDir path: '" << path << "'";
        auto [parent, name] = getParentAndName(path);

        if (!parent || name.empty()) {
            LOG_ERROR << "无效路径: " << path;
            return false;
        }

        LOG_DEBUG << "createDir - parent path: " << (parent == &root_json ? "ROOT" : "CHILD") << ", name: '" << name << "'";

        // 检查目录是否已存在
        if (parent == &root_json) {
            if (parent->contains(name)) {
                LOG_DEBUG << "目录已存在: " << path;
                return true;
            }
        } else {
            if (parent->contains("children") && (*parent)["children"].contains(name)) {
                LOG_DEBUG << "目录已存在: " << path;
                return true;
            }
        }

        // 创建新目录
        json new_dir;
        new_dir["is_dir"] = true;
        new_dir["children"] = json::object();

        if (parent == &root_json) {
            // 直接在根目录下创建
            LOG_DEBUG << "Creating directory in root: " << name;
            (*parent)[name] = new_dir;
        } else {
            // 在children中创建
            LOG_DEBUG << "Creating directory in children: " << name;
            (*parent)["children"][name] = new_dir;
        }

        LOG_DEBUG << "About to save JSON content";
        saveToFile(this->file_path);
        LOG_DEBUG << "创建目录: " << path;
        return true;
    }
    
    // 添加文件
    bool addFile(const std::string& path, const std::string& token = "", size_t file_size = 0) {
        LOG_DEBUG << "addFile path: '" << path << "'";
        auto [parent, name] = getParentAndName(path);

        if (!parent || name.empty()) {
            LOG_ERROR << "无效路径: " << path;
            return false;
        }

        LOG_DEBUG << "addFile - parent path: " << (parent == &root_json ? "ROOT" : "CHILD") << ", name: '" << name << "'";

        // 检查是否已存在
        if (parent == &root_json) {
            if (parent->contains(name)) {
                LOG_DEBUG << "文件已存在: " << path;
                return true;
            }
        } else {
            if (parent->contains("children") && (*parent)["children"].contains(name)) {
                LOG_DEBUG << "文件已存在: " << path;
                return true;
            }
        }

        // 创建新文件
        json new_file;
        new_file["is_dir"] = false;
        new_file["token"] = token;
        new_file["file_size"] = file_size;

        if (parent == &root_json) {
            // 直接在根目录下创建
            LOG_DEBUG << "Adding file in root: " << name;
            (*parent)[name] = new_file;
        } else {
            // 在children中创建
            LOG_DEBUG << "Adding file in children: " << name;
            (*parent)["children"][name] = new_file;
        }

        LOG_DEBUG << "About to save JSON content for file";
        saveToFile(this->file_path);
        LOG_DEBUG << "添加文件: " << path << " (token: " << token << ")";
        return true;
    }
    
    // 删除文件或目录
    bool remove(const std::string& path) {
        auto [parent, name] = getParentAndName(path);

        if (!parent || name.empty()) {
            LOG_ERROR << "路径不存在: " << path;
            return false;
        }

        // 检查父节点是否为根目录
        if (parent == &root_json) {
            if (!parent->contains(name)) {
                LOG_ERROR << "路径不存在: " << path;
                return false;
            }

            json& node = (*parent)[name];
            // 如果是目录，递归删除
            if (node.contains("is_dir") && node["is_dir"]) {
                removeDirectory(node);
            }

            // 从根节点中删除
            (*parent).erase(name);
        } else {
            if (!parent->contains("children") || !(*parent)["children"].contains(name)) {
                LOG_ERROR << "路径不存在: " << path;
                return false;
            }

            json& node = (*parent)["children"][name];
            // 如果是目录，递归删除
            if (node.contains("is_dir") && node["is_dir"]) {
                removeDirectory(node);
            }

            // 从父节点中删除
            (*parent)["children"].erase(name);
        }

        saveToFile(this->file_path);
        LOG_DEBUG << "删除: " << path;
        return true;
    }
    
    // 重命名文件或目录
    bool rename(const std::string& old_path, const std::string& new_name) {
        LOG_DEBUG << "rename old_path: '" << old_path << "' new_name: '" << new_name << "'";
        auto [parent, old_name] = getParentAndName(old_path);

        if (!parent || old_name.empty()) {
            LOG_ERROR << "原路径不存在: " << old_path;
            return false;
        }

        LOG_DEBUG << "rename - old_name: '" << old_name << "' new_name: '" << new_name << "'";
        LOG_DEBUG << "rename - parent path: " << (parent == &root_json ? "ROOT" : "CHILD");

        // 检查父节点是否为根目录
        if (parent == &root_json) {
            if (!parent->contains(old_name)) {
                LOG_ERROR << "原路径不存在: " << old_path;
                return false;
            }

            if (parent->contains(new_name)) {
                LOG_ERROR << "新名称已存在: " << new_name;
                return false;
            }

            // 移动节点到新名称
            json node = (*parent)[old_name];
            (*parent).erase(old_name);
            (*parent)[new_name] = node;
            LOG_DEBUG << "Renamed in root: " << old_name << " -> " << new_name;
        } else {
            if (!parent->contains("children") || !(*parent)["children"].contains(old_name)) {
                LOG_ERROR << "原路径不存在: " << old_path;
                return false;
            }

            if ((*parent)["children"].contains(new_name)) {
                LOG_ERROR << "新名称已存在: " << new_name;
                return false;
            }

            // 移动节点到新名称
            json node = (*parent)["children"][old_name];
            (*parent)["children"].erase(old_name);
            (*parent)["children"][new_name] = node;
            LOG_DEBUG << "Renamed in children: " << old_name << " -> " << new_name;
        }

        LOG_DEBUG << "About to save JSON content for rename";
        saveToFile(this->file_path);
        LOG_DEBUG << "重命名: " << old_path << " -> " << new_name;
        return true;
    }
    
    // 移动文件或目录
    bool move(const std::string& src_path, const std::string& dest_dir) {
        auto [src_parent, src_name] = getParentAndName(src_path);
        
        if (!src_parent || src_name.empty() || !src_parent->contains(src_name)) {
            LOG_ERROR << "源路径不存在: " << src_path;
            return false;
        }
        
        json* dest_parent = getNodeAtPath(dest_dir, false);
        if (!dest_parent || !dest_parent->contains("is_dir") || !(*dest_parent)["is_dir"]) {
            LOG_ERROR << "目标目录不存在: " << dest_dir;
            return false;
        }
        
        // 检查目标目录中是否有同名文件
        if ((*dest_parent)["children"].contains(src_name)) {
            LOG_ERROR << "目标目录已存在同名文件: " << src_name;
            return false;
        }
        
        // 获取源节点
        json src_node = (*src_parent)[src_name];
        
        // 从源父节点中删除
        src_parent->erase(src_name);
        
        // 添加到目标目录
        (*dest_parent)["children"][src_name] = src_node;
        
        // std::cout << "移动: " << src_path << " -> " << dest_dir << std::endl;
        LOG_DEBUG << "移动: " << src_path << " -> " << dest_dir;
        saveToFile(this->file_path);
        return true;
    }
    
    std::string getFileToken(const std::string& path) {
        auto file_node = getFile(path);
        if (file_node.name.empty() || file_node.is_dir) {
            LOG_ERROR << "文件不存在: " << path;
            return "";
        }
        if (file_node.token == "PENDING") {
            LOG_DEBUG << "文件正在写入中: " << path;
            return "";
        }
        return file_node.token;
    }

    // 显示当前目录结构（用于调试）
    void printStructure(const std::string& prefix = "", json* node = nullptr) {
        if (!node) node = &root_json;

        for (auto it = node->begin(); it != node->end(); ++it) {
            std::cout << prefix << it.key();

            if (it.value().contains("is_dir") && it.value()["is_dir"]) {
                std::cout << "/" << std::endl;
                if (it.value().contains("children")) {
                    printStructure(prefix + "  ", &it.value()["children"]);
                }
            } else {
                std::cout << " (token: "
                         << (it.value().contains("token") ? it.value()["token"].get<std::string>() : "")
                         << ", size: "
                         << (it.value().contains("file_size") ? std::to_string(it.value()["file_size"].get<size_t>()) : "0")
                         << ")" << std::endl;
            }
        }
    }

    // 验证JSON文件的完整性
    bool validateJSONFile() {
        if (file_path.empty()) {
            LOG_ERROR << "File path is empty";
            return false;
        }

        LOG_DEBUG << "Validating JSON file: " << file_path;

        // 检查文件是否存在
        if (!std::filesystem::exists(file_path)) {
            LOG_INFO << "JSON file does not exist: " << file_path;
            return false;
        }

        std::ifstream file(file_path);
        if (!file.is_open()) {
            LOG_ERROR << "Cannot open JSON file: " << file_path;
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        LOG_DEBUG << "JSON file content: '" << content << "' (size: " << content.size() << " bytes)";

        if (content.empty()) {
            LOG_INFO << "JSON file is empty: " << file_path;
            return false;
        }

        try {
            json test_json = json::parse(content);
            LOG_DEBUG << "JSON file is valid, size: " << content.size() << " bytes";
            return true;
        } catch (const json::parse_error& e) {
            LOG_ERROR << "JSON file is malformed: " << e.what() << " Content: " << content;
            return false;
        }
    }
};

// 测试函数
// void testFileManager() {
    // FileManager fm;
    
    // std::cout << "=== 测试1: 创建示例JSON结构 ===" << std::endl;
    
    // // 创建目录结构
    // fm.createDir("/dir1");
    // fm.addFile("/dir1/file1.txt", "test1");
    // fm.createDir("/dir1/subdir1");
    // fm.addFile("/file2.txt", "test2");
    
    // // 列出目录
    // std::cout << "\n=== 测试2: 列出目录 ===" << std::endl;
    // auto dir1_contents = fm.listDir("/dir1");
    // for (const auto& node : dir1_contents) {
    //     std::cout << "名称: " << node.name 
    //               << ", 类型: " << (node.is_dir ? "目录" : "文件") 
    //               << (node.is_dir ? "" : ", token: " + node.token) 
    //               << std::endl;
    // }
    
    // // 获取文件信息
    // std::cout << "\n=== 测试3: 获取文件信息 ===" << std::endl;
    // FileNode file2 = fm.getFile("/file2.txt");
    // if (!file2.name.empty()) {
    //     std::cout << "文件: " << file2.name << ", token: " << file2.token << std::endl;
    // }
    
//    // 添加更多文件和目录
//     std::cout << "\n=== 测试4: 添加更多内容 ===" << std::endl;
//     fm.addFile("/dir1/file3.txt", "test3");
//     fm.createDir("/dir1/subdir2");
//     fm.addFile("/dir1/subdir2/nested.txt", "nested_token");
    
//     // 重命名测试
//     std::cout << "\n=== 测试5: 重命名 ===" << std::endl;
//     fm.rename("/dir1/file3.txt", "renamed_file.txt");
    
//     // 移动测试
//     std::cout << "\n=== 测试6: 移动文件 ===" << std::endl;
//     fm.move("/file2.txt", "/dir1");
    
//     // 删除测试
//     std::cout << "\n=== 测试7: 删除文件 ===" << std::endl;
//     fm.remove("/dir1/renamed_file.txt");
    
//     // 显示最终结构
//     std::cout << "\n=== 最终目录结构 ===" << std::endl;
//     fm.printStructure();
    
//     // 保存到文件
//     std::cout << "\n=== 保存到文件 ===" << std::endl;
//     fm.saveToFile("filesystem.json");
    
//     // 从文件加载
//     std::cout << "\n=== 从文件加载 ===" << std::endl;
//     FileManager fm2;
//     if (fm2.loadFromFile("filesystem.json")) {
//         std::cout << "加载后的目录结构:" << std::endl;
//         fm2.printStructure();
//     }
// }