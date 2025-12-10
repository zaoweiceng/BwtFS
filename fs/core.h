#ifndef MY_FS_CORE_H
#define MY_FS_CORE_H

#include <string>
#include <unordered_map>
#include <vector>
#include "manager.hpp"

class MemoryFS {
public:
    struct File {
        std::string name;
        std::vector<char> data;
        bool is_directory;  // 标识是否为目录
    };

    int open(const std::string& path);
    int read(int fd, char* buf, size_t size);
    int read(int fd, char* buf, size_t size, off_t offset);
    int write(int fd, const char* buf, size_t size);
    int write(int fd, const char* buf, size_t size, off_t offset);
    int remove(const std::string& path);
    int close(int fd);
    int create(const std::string& path);
    int mkdir(const std::string& path);
    int rename(const std::string& old_path, const std::string& new_path);
    std::vector<std::string> list_files();
    std::vector<std::string> list_files_in_dir(const std::string& dir_path);
    bool is_directory(const std::string& path);
    std::string normalize_path(const std::string& path);
    std::string get_parent_dir(const std::string& path);
    std::string get_basename(const std::string& path);
    void remove_recursive(const std::string& path);
    void move_recursive(const std::string& old_path, const std::string& new_path);
    std::unordered_map<std::string, File> files_;   // 文件名 -> 文件数据
private:
    
    std::unordered_map<int, std::string> fd_map_;   // 文件描述符 -> 文件名
    int next_fd_ = 1;
};

class BwtFSMounter {
    private:
        SystemManager system_manager_;
        FileManager file_manager_;
        MemoryFS memory_fs_;  // 用于存储系统临时文件的内存文件系统
        std::unordered_map<int, std::string> fd_map_;   // 文件描述符 -> 文件路径
        std::unordered_map<std::string, int> path_fd_map_; // 文件路径 -> 主文件描述符
        std::unordered_map<std::string, int> path_ref_count_; // 文件路径 -> 引用计数
        std::unordered_map<int, BwtFS::Node::bw_tree*> fd_tree_map_; // 文件描述符 -> bw_tree对象
        // std::unordered_map<int, BwtFS::Node::bw_tree*> fd_write_wait_map_; // 文件描述符 -> 写入用bw_tree对象
        // std::unordered_map<int, size_t> fd_file_size_map_; // 文件描述符 -> 文件大小
        std::unordered_map<int, int> fd_to_memory_fd_map_; // BwtFS文件描述符 -> 内存文件描述符
        int next_fd_ = 1;

        // 判断文件是否应该存储在memory_fs中（系统临时文件）
        bool isSystemTempFile(const std::string& path) {
            std::string basename = path.substr(path.find_last_of('/') + 1);

            // macOS系统文件
            // if (basename.find("._") == 0 ||               // 资源分支文件
            //     basename == ".DS_Store" ||                 // 桌面服务存储文件
            //     basename == "Thumbs.db" ||                 // Windows缩略图缓存
            //     basename == "desktop.ini" ||               // Windows桌面配置
            //     basename.find(".TemporaryItems") == 0 ||   // macOS临时项目
            //     basename.find(".vscode") == 0 ||           // VSCode临时文件
            //     basename.find("~$") == 0) {                // Office临时文件
            //     return true;
            // }

            return false;
        }
    public:
        BwtFSMounter(std::string system_file_path, std::string initial_dir_path){
            system_manager_.init(system_file_path);
            file_manager_ = FileManager(initial_dir_path);

            // 验证JSON文件的完整性
            if (!file_manager_.validateJSONFile()) {
                LOG_WARNING << "JSON file validation failed, attempting to create empty structure";
                // 如果JSON文件无效，创建一个空的JSON结构
                std::ofstream empty_json(initial_dir_path);
                if (empty_json.is_open()) {
                    empty_json << "{}";
                    empty_json.close();
                    file_manager_.loadFromFile(initial_dir_path);
                    LOG_INFO << "Created empty JSON structure";
                } else {
                    LOG_ERROR << "Failed to create empty JSON file";
                }
            }
        }
        BwtFSMounter(){}
        ~BwtFSMounter(){
        }
        void init(std::string system_file_path, std::string initial_dir_path){
            system_manager_ = SystemManager(system_file_path);
            file_manager_ = FileManager(initial_dir_path);

            // 验证JSON文件的完整性
            if (!file_manager_.validateJSONFile()) {
                LOG_WARNING << "JSON file validation failed, attempting to create empty structure";
                // 如果JSON文件无效，创建一个空的JSON结构
                std::ofstream empty_json(initial_dir_path);
                if (empty_json.is_open()) {
                    empty_json << "{}";
                    empty_json.close();
                    file_manager_.loadFromFile(initial_dir_path);
                    LOG_INFO << "Created empty JSON structure";
                } else {
                    LOG_ERROR << "Failed to create empty JSON file";
                }
            }
        }
        int open(const std::string& path, int flags = 0);
        int read(int fd, char* buf, size_t size);
        int read(int fd, char* buf, size_t size, off_t offset);
        int write(int fd, const char* buf, size_t size);
        int write(int fd, const char* buf, size_t size, off_t offset);
        int remove(const std::string& path);
        int close(int fd);
        int create(const std::string& path);
        int mkdir(const std::string& path);
        int rename(const std::string& old_path, const std::string& new_path);
        std::vector<std::string> list_files();
        std::vector<std::string> list_files_in_dir(const std::string& dir_path);
        bool is_directory(const std::string& path);
        std::string normalize_path(const std::string& path);
        std::string get_parent_dir(const std::string& path);
        std::string get_basename(const std::string& path);
        void remove_recursive(const std::string& path);
        void move_recursive(const std::string& old_path, const std::string& new_path);
        bool file_exists(const std::string& path);
        FileNode getFileNode(const std::string& path);
        SystemInfo getSystemInfo();
        void cleanupFdMappings(int fd);
};
#endif // MY_FS_CORE_H