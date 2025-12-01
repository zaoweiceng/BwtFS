#ifndef MY_FS_CORE_H
#define MY_FS_CORE_H

#include <string>
#include <unordered_map>
#include <vector>

class MyFS {
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

#endif // MY_FS_CORE_H