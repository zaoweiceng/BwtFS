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
    int next_fd_ = 3;
};

class BwtFSMounter {
    private:
        SystemManager system_manager_;
        FileManager file_manager_;
        std::unordered_map<int, std::string> fd_map_;   // 文件描述符 -> 文件路径
        std::unordered_map<std::string, int> path_fd_map_; // 文件路径 -> 文件描述符
        std::unordered_map<int, BwtFS::Node::bw_tree*> fd_tree_map_; // 文件描述符 -> bw_tree对象
        std::unordered_map<int, BwtFS::Node::bw_tree*> fd_write_wait_map_; // 文件描述符 -> 写入用bw_tree对象
        std::unordered_map<int, size_t> fd_file_size_map_; // 文件描述符 -> 文件大小
        int next_fd_ = 1;
    public:
        BwtFSMounter(std::string system_file_path, std::string initial_dir_path){
            system_manager_.init(system_file_path);
            file_manager_ = FileManager(initial_dir_path);
        }
        BwtFSMounter(){}
        ~BwtFSMounter(){
        }
        void init(std::string system_file_path, std::string initial_dir_path){
            system_manager_ = SystemManager(system_file_path);
            file_manager_ = FileManager(initial_dir_path);
        }
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
        bool file_exists(const std::string& path);
        FileNode getFileNode(const std::string& path);
        SystemInfo getSystemInfo();
};
#endif // MY_FS_CORE_H