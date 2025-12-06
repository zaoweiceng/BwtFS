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
        SystemManager(std::string system_path, std::string dir_path){
            if(!fileExists(system_path)){
                LOG_ERROR << "BwtFS system file does not exist: " << system_path;
                throw std::runtime_error("BwtFS system file does not exist: " + system_path);
            }
            if (!fileExists(dir_path)){
                LOG_ERROR << "Directory path does not exist: " << dir_path;
                throw std::runtime_error("Directory path does not exist: " + dir_path);
            }
            filesystem_ = BwtFS::System::openBwtFS(system_path);

        }
        ~SystemManager(){
            filesystem_->~FileSystem();
        }
        // getSystemInfo
        SystemInfo getSystemInfo(){
            SystemInfo info;
            info.file_size = filesystem_->getFileSize();
            info.block_size = filesystem_->getBlockSize();
            info.block_count = filesystem_->getBlockCount();
            info.create_time = filesystem_->getCreateTime();
            info.modify_time = filesystem_->getModifyTime();
            return info;
        }
};

// class FileNode{
//     private:
//         bool is_directory;
//         json* content;
//         std::vector<std::string> file_list;
//         std::string file_token;
//         std::map<std::string, std::shared_ptr<json*>> data;
//         void readDataFromJson(){
//             if (is_directory){
//                 for (auto& el : content->items()){
//                     file_list.push_back(el.key());
//                     data[el.key()] = std::make_shared<json*>(&(*content)[el.key()]);
//                 }
//             }else{
//                 file_token = content->get<std::string>();
//             }
//         };
//     public:
//         FileNode(bool is_directory, json* content): is_directory(is_directory), content(content){
//             readDataFromJson();
//         };
//         ~FileNode(){};
//         bool isDirectory() const {
//             return is_directory;
//         }
//         json* getContent() const {
//             return content;
//         }
//         std::string toString() const {
//             if(content->is_null()){
//                 return "";
//             }
//             return content->dump(4);
//         }
//         json toJson() {
//             json res_data;
//             if (!is_directory){
//                 res_data = {
//                     "token", file_token
//                 };
//             }else{
//                 res_data = {
//                     "files", json::array()
//                 };
//                 for(auto chile_node : data){
//                     res_data["files"].push_back({
//                         "name", chile_node.first,
//                         "is_directory", (*chile_node.second)->is_object()
//                     });
//                 }
//             }
//             return res_data;
//         }
//         std::vector<std::string> getFileList() const {
//             return file_list;
//         }
        
        
// };

// class NodeManager{
//     private:
//         json data;
//         std::string dir_file_path;

//         bool loadDirectoryFromFileByPath(const std::string& dir_file_path){
//             std::ifstream infile(dir_file_path);
//             if (!infile.is_open()){
//                 LOG_ERROR << "Failed to open directory file: " << dir_file_path;
//                 return false;
//             }
//             try{
//                 infile >> data;
//             }catch (const std::exception& e){
//                 LOG_ERROR << "Failed to parse directory file: " << dir_file_path << " Error: " << e.what();
//                 infile.close();
//                 return false;
//             }
//             infile.close();
//             return true;
//         }
//     public:
//         NodeManager(std::string dir_file_path){
//             if(!std::filesystem::exists(dir_file_path)){
//                 LOG_ERROR << "Directory file does not exist: " << dir_file_path;
//                 throw std::runtime_error("Directory file does not exist: " + dir_file_path);
//             }
//             loadDirectoryFromFileByPath(dir_file_path);
//         }
//         ~NodeManager(){};
//         void clearData(){
//             data = json::object();
//         }
//         bool saveDirectoryToFileByPath(const std::string& dir_file_path){
//             std::ofstream outfile(dir_file_path);
//             if (!outfile.is_open()){
//                 LOG_ERROR << "Failed to open directory file for writing: " << dir_file_path;
//                 return false;
//             }
//             try{
//                 if (data.is_null()){
//                     data = json::object();
//                 }
//                 outfile << data.dump(4);
//             }catch (const std::exception& e){
//                 LOG_ERROR << "Failed to write directory file: " << dir_file_path << " Error: " << e.what();
//                 outfile.close();
//                 return false;
//             }
//             outfile.close();
//             return true;
//         }
//         // getDirectoryNodeByPath
//         bool createDirectoryByPath(std::string parent_path, std::string dir_name){
//             json* current = &data;
//             if (parent_path != "/"){
//                 std::istringstream ss(parent_path);
//                 std::string token;
//                 while (std::getline(ss, token, '/')){
//                     if (token.empty()) continue;
//                     if (current->contains(token)){
//                         current = &(*current)[token];
//                     }else{
//                         LOG_ERROR << "Parent directory does not exist: " << parent_path;
//                         return false;
//                     }
//                 }
//             }
//             if (current->contains(dir_name)){
//                 LOG_ERROR << "Directory already exists: " << dir_name;
//                 return false;
//             }
//             (*current)[dir_name] = json::object();
//             return true;
//         }
//         bool deleteDirectoryByPath(std::string dir_path){
//             json* current = &data;
//             std::istringstream ss(dir_path);
//             std::string token;
//             std::vector<json*> path_nodes;
//             path_nodes.push_back(current);
//             while (std::getline(ss, token, '/')){
//                 if (token.empty()) continue;
//                 if (current->contains(token)){
//                     current = &(*current)[token];
//                     path_nodes.push_back(current);
//                 }else{
//                     LOG_ERROR << "Directory does not exist: " << dir_path;
//                     return false;
//                 }
//                 LOG_INFO << "Traversing to: " << token;
//             }
//             // 删除节点
//             if (path_nodes.size() < 2){
//                 LOG_ERROR << "Cannot delete root directory";
//                 return false;
//             }
//             json* parent = path_nodes[path_nodes.size() - 2];
//             parent->erase(token);
//             return true;
//         }
//         bool renameDirectoryByPath(std::string dir_path, std::string new_name){
//             json* current = &data;
//             std::istringstream ss(dir_path);
//             std::string token;
//             std::vector<json*> path_nodes;
//             path_nodes.push_back(current);
//             while (std::getline(ss, token, '/')){
//                 if (token.empty()) continue;
//                 if (current->contains(token)){
//                     current = &(*current)[token];
//                     path_nodes.push_back(current);
//                 }else{
//                     LOG_ERROR << "Directory does not exist: " << dir_path;
//                     return false;
//                 }
//             }
//             // 重命名节点
//             if (path_nodes.size() < 2){
//                 LOG_ERROR << "Cannot rename root directory";
//                 return false;
//             }
//             json* parent = path_nodes[path_nodes.size() - 2];
//             (*parent)[new_name] = *current;
//             parent->erase(token);
//             return true;
//         }
//         // 将一个文件夹(包括文件夹内部的内容)从old_path移动到new_path目录下
//         bool moveDirectoryByPath(std::string old_path, std::string new_path){
//             auto normalize = [](std::string p)->std::string{
//                 if (p.empty()) return "/";
//                 // remove trailing slashes (but keep single leading slash for root)
//                 while (p.size() > 1 && p.back() == '/') p.pop_back();
//                 if (p.front() != '/') p = "/" + p;
//                 return p;
//             };
//             old_path = normalize(old_path);
//             new_path = normalize(new_path);

//             if (old_path == "/"){
//                 LOG_ERROR << "Cannot move root directory";
//                 return false;
//             }
//             // Prevent moving into itself or its descendant
//             if (new_path == old_path || (new_path.size() > old_path.size() && new_path.rfind(old_path, 0) == 0 && new_path[old_path.size()] == '/')){
//                 LOG_ERROR << "Cannot move directory into itself or its own subtree: " << old_path << " -> " << new_path;
//                 return false;
//             }

//             // Locate source node and its parent
//             json* current = &data;
//             std::istringstream ss(old_path);
//             std::string token;
//             std::vector<json*> path_nodes;
//             path_nodes.push_back(current);
//             std::string last_token;
//             while (std::getline(ss, token, '/')){
//                 if (token.empty()) continue;
//                 if (current->contains(token)){
//                     current = &(*current)[token];
//                     path_nodes.push_back(current);
//                     last_token = token;
//                 } else {
//                     LOG_ERROR << "Source directory does not exist: " << old_path;
//                     return false;
//                 }
//             }
//             if (path_nodes.size() < 2){
//                 LOG_ERROR << "Cannot move root directory";
//                 return false;
//             }
//             json* src_parent = path_nodes[path_nodes.size() - 2];

//             // Locate destination node
//             json* dest = &data;
//             if (new_path != "/"){
//                 std::istringstream ss2(new_path);
//                 while (std::getline(ss2, token, '/')){
//                     if (token.empty()) continue;
//                     if (dest->contains(token)){
//                         dest = &(*dest)[token];
//                     } else {
//                         LOG_ERROR << "Destination directory does not exist: " << new_path;
//                         return false;
//                     }
//                 }
//             }

//             // Prevent name collision at destination
//             if (dest->contains(last_token)){
//                 LOG_ERROR << "Destination already contains a directory named: " << last_token;
//                 return false;
//             }

//             // Move: copy node to destination and erase from source parent
//             (*dest)[last_token] = *current;
//             src_parent->erase(last_token);
//             return true;
//         }
// };

// 文件节点信息结构体
struct FileNode {
    std::string name;
    bool is_dir;
    std::string token;  // 仅文件有值
    json* node_ptr;     // 指向原始JSON节点的指针（用于惰性解析）
    
    FileNode(const std::string& n = "", bool d = false, const std::string& t = "", json* ptr = nullptr)
        : name(n), is_dir(d), token(t), node_ptr(ptr) {}
};

// 文件目录管理类
class FileManager {
private:
    json root_json;               // 完整的JSON结构
    
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
        
        // 空路径或根路径
        if (components.empty()) {
            return &root_json;
        }
        
        json* current = &root_json;
        
        for (size_t i = 0; i < components.size(); i++) {
            const std::string& component = components[i];
            
            // 如果节点不存在
            if (!current->contains(component)) {
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
                } else {
                    return nullptr;  // 节点不存在
                }
            } else {
                // 节点存在，解析其is_dir
                if (!(*current)[component].contains("is_dir")) {
                    return nullptr;  // 无效节点结构
                }
                
                bool is_dir = (*current)[component]["is_dir"];
                
                if (is_dir) {
                    // 如果是目录且不是最后一个组件，进入children
                    if (i < components.size() - 1) {
                        current = &(*current)[component]["children"];
                    }
                } else {
                    // 如果是文件，只能作为最后一个组件
                    if (i < components.size() - 1) {
                        return nullptr;  // 试图访问文件的子节点
                    }
                    current = &(*current)[component];
                }
            }
        }
        
        return  &(*current)[components.back()]; // 返回最终节点
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
        std::string parent_path = "";
        for (const auto& comp : components) {
            parent_path += "/" + comp;
        }
        LOG_DEBUG << "Parent path: " << parent_path << ", name: " << name;
        json* parent = getNodeAtPath(parent_path);
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
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR << "加载JSON文件失败: " << e.what();
            return false;
        }
    }
    
    // 保存到文件
    bool saveToFile(const std::string& filename) {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) {
                LOG_ERROR << "无法创建文件: " << filename;
                return false;
            }
            
            file << root_json.dump(4);  // 缩进为4个空格
            LOG_DEBUG << "成功保存JSON文件: " << filename;
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR << "保存JSON文件失败: " << e.what();
            return false;
        }
    }
    
    // 列出目录内容（惰性解析）
    std::vector<FileNode> listDir(const std::string& path) {
        std::vector<FileNode> result;
        
        json* node = getNodeAtPath(path);
        if (!node || !node->contains("is_dir") || !(*node)["is_dir"]) {
            LOG_ERROR << node->dump();
            LOG_ERROR << "路径不存在或不是目录: " << path;
            return result;
        }
        
        // 检查是否有children字段
        if (!node->contains("children") || !(*node)["children"].is_object()) {
            LOG_ERROR << "目录结构无效: " << path;
            return result;
        }
        
        // 遍历children，只解析当前层级
        for (auto it = (*node)["children"].begin(); it != (*node)["children"].end(); ++it) {
            const std::string& name = it.key();
            if (it.value().contains("is_dir")) {
                bool is_dir = it.value()["is_dir"];
                std::string token = is_dir ? "" : it.value()["token"];
                result.emplace_back(name, is_dir, token, &it.value());
            }
        }
        
        return result;
    }
    
    // 获取文件信息
    FileNode getFile(const std::string& path) {
        auto [parent, name] = getParentAndName(path);
        
        if (!parent || name.empty() || !parent->contains(name)) {
            LOG_ERROR << "文件不存在: " << path;
            return FileNode();
        }
        
        json& node = (*parent)[name];
        if (!node.contains("is_dir") || node["is_dir"]) {
            LOG_ERROR << "路径不是文件: " << path;
            return FileNode();
        }
        
        std::string token = node.contains("token") ? node["token"] : "";
        return FileNode(name, false, token, &node);
    }
    
    // 创建目录
    bool createDir(const std::string& path) {
        auto [parent, name] = getParentAndName(path);
        
        if (!parent || name.empty()) {
            LOG_ERROR << "无效路径: " << path;
            return false;
        }
        
        if (parent->contains(name)) {
            LOG_ERROR << "目录已存在: " << path;
            return false;
        }
        
        // 创建新目录
        json new_dir;
        new_dir["is_dir"] = true;
        new_dir["children"] = json::object();
        if (splitPath(path).size() == 1){
            (*parent)[name] = new_dir;
        }else{
            (*parent)["children"][name] = new_dir;
        }
        LOG_DEBUG << "创建目录: " << path;
        return true;
    }
    
    // 添加文件
    bool addFile(const std::string& path, const std::string& token = "") {
        auto [parent, name] = getParentAndName(path);
        
        if (!parent || name.empty()) {
            LOG_ERROR << "无效路径: " << path;
            return false;
        }
        
        if (parent->contains(name)) {
            LOG_ERROR << "文件已存在: " << path;
            return false;
        }
        
        // 创建新文件
        json new_file;
        new_file["is_dir"] = false;
        new_file["token"] = token;
        if (splitPath(path).size() == 1){
            (*parent)[name] = new_file;
        }else{
            (*parent)["children"][name] = new_file;
        }
        // LOG_DEBUG << "parent: " << parent->dump();
        // LOG_DEBUG << "name: " << name;
        // LOG_DEBUG << "new_file: " << new_file.dump();    
        LOG_DEBUG << "添加文件: " << path << " (token: " << token << ")";
        return true;
    }
    
    // 删除文件或目录
    bool remove(const std::string& path) {
        auto [parent, name] = getParentAndName(path);
        
        if (!parent || name.empty() || !(*parent)["children"].contains(name)) {
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
        
        // std::cout << "删除: " << path << std::endl;
        LOG_DEBUG << "删除: " << path;
        return true;
    }
    
    // 重命名文件或目录
    bool rename(const std::string& old_path, const std::string& new_name) {
        auto [parent, old_name] = getParentAndName(old_path);
        // LOG_DEBUG << "Renaming path: " << old_path << " to new name: " << new_name << ", old name: " << old_name;
        // LOG_DEBUG << "Parent node: " << (parent ? parent->dump() : "null");
        if (!parent || old_name.empty() || !(*parent)["children"].contains(old_name)) {
            LOG_ERROR << "原路径不存在: " << old_path;
            return false;
        }
        
        if ((*parent)["children"].contains(new_name)) {
            LOG_ERROR << "新名称已存在: " << new_name;
            return false;
        }
        
        // 移动节点到新名称
        json node = (*parent)["children"][old_name];
        // parent->erase(old_name);
        (*parent)["children"].erase(old_name);
        // (*parent)[new_name] = node;
        (*parent)["children"][new_name] = node;
        
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
        return true;
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
                         << ")" << std::endl;
            
            }
        }
    }
};

// 测试函数
void testFileManager() {
    FileManager fm;
    
    std::cout << "=== 测试1: 创建示例JSON结构 ===" << std::endl;
    
    // 创建目录结构
    fm.createDir("/dir1");
    fm.addFile("/dir1/file1.txt", "test1");
    fm.createDir("/dir1/subdir1");
    fm.addFile("/file2.txt", "test2");
    
    // 列出目录
    std::cout << "\n=== 测试2: 列出目录 ===" << std::endl;
    auto dir1_contents = fm.listDir("/dir1");
    for (const auto& node : dir1_contents) {
        std::cout << "名称: " << node.name 
                  << ", 类型: " << (node.is_dir ? "目录" : "文件") 
                  << (node.is_dir ? "" : ", token: " + node.token) 
                  << std::endl;
    }
    
    // 获取文件信息
    std::cout << "\n=== 测试3: 获取文件信息 ===" << std::endl;
    FileNode file2 = fm.getFile("/file2.txt");
    if (!file2.name.empty()) {
        std::cout << "文件: " << file2.name << ", token: " << file2.token << std::endl;
    }
    
    // 添加更多文件和目录
    std::cout << "\n=== 测试4: 添加更多内容 ===" << std::endl;
    fm.addFile("/dir1/file3.txt", "test3");
    fm.createDir("/dir1/subdir2");
    fm.addFile("/dir1/subdir2/nested.txt", "nested_token");
    
    // 重命名测试
    std::cout << "\n=== 测试5: 重命名 ===" << std::endl;
    fm.rename("/dir1/file3.txt", "renamed_file.txt");
    
    // 移动测试
    std::cout << "\n=== 测试6: 移动文件 ===" << std::endl;
    fm.move("/file2.txt", "/dir1");
    
    // 删除测试
    std::cout << "\n=== 测试7: 删除文件 ===" << std::endl;
    fm.remove("/dir1/renamed_file.txt");
    
    // 显示最终结构
    std::cout << "\n=== 最终目录结构 ===" << std::endl;
    fm.printStructure();
    
    // 保存到文件
    std::cout << "\n=== 保存到文件 ===" << std::endl;
    fm.saveToFile("filesystem.json");
    
    // 从文件加载
    std::cout << "\n=== 从文件加载 ===" << std::endl;
    FileManager fm2;
    if (fm2.loadFromFile("filesystem.json")) {
        std::cout << "加载后的目录结构:" << std::endl;
        fm2.printStructure();
    }
}