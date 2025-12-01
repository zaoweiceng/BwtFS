#include "core.h"
#include <cstring>
#include <iostream>

int MyFS::open(const std::string& path){
    if (files_.find(path) == files_.end()) {
        // 如果文件不存在，则创建为普通文件（保持向后兼容）
        files_[path] = File{path, {}, false};
    }
    int fd = next_fd_++;
    fd_map_[fd] = path;
    std::cout << "[open] " << path << " -> fd=" << fd << std::endl;
    return fd;
}

int MyFS::create(const std::string& path) {
    // 如果文件不存在，则创建一个空文件
    if (files_.find(path) == files_.end()) {
        files_[path] = File{path, {}, false};  // 普通文件
        std::cout << "[create] " << path << std::endl;
        return 0;
    }
    // 已经存在也返回 0，表示成功（类似 FUSE 语义）
    return 0;
}


int MyFS::read(int fd, char* buf, size_t size){
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto& f = files_[it->second];

    size = std::min(size, f.data.size());
    memcpy(buf, f.data.data(), size);
    std::cout << "[read] fd=" << fd << " size=" << size << std::endl;
    return size;
}

int MyFS::read(int fd, char* buf, size_t size, off_t offset) {
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto& f = files_[it->second];

    if (offset >= static_cast<off_t>(f.data.size())) {
        return 0; // EOF
    }

    size = std::min<size_t>(size, f.data.size() - offset);
    memcpy(buf, f.data.data() + offset, size);
    std::cout << "[read] fd=" << fd << " offset=" << offset << " size=" << size << std::endl;
    return size;
}


int MyFS::write(int fd, const char* buf, size_t size){
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto& f = files_[it->second];

    f.data.assign(buf, buf + size);
    std::cout << "[write] fd=" << fd << " size=" << size << std::endl;
    return size;
}

int MyFS::write(int fd, const char* buf, size_t size, off_t offset) {
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto& f = files_[it->second];

    // 如果写入位置超过当前文件长度，先扩展
    if (offset + size > f.data.size()) {
        f.data.resize(offset + size);
    }

    memcpy(f.data.data() + offset, buf, size);
    std::cout << "[write] fd=" << fd << " offset=" << offset << " size=" << size << std::endl;
    return size;
}


int MyFS::remove(const std::string& path){
    std::cout << "[unlink] " << path << std::endl;
    return files_.erase(path) ? 0 : -1;
}

int MyFS::close(int fd){
    std::cout << "[close] fd=" << fd << std::endl;
    return fd_map_.erase(fd) ? 0 : -1;
}

int MyFS::mkdir(const std::string& path) {
    // 如果目录不存在，则创建一个空目录
    if (files_.find(path) == files_.end()) {
        files_[path] = File{path, {}, true};   // 目录
        std::cout << "[mkdir] " << path << std::endl;
        return 0;
    }
    // 已经存在，如果是目录则返回成功，否则返回错误
    return files_[path].is_directory ? 0 : -1;
}

int MyFS::rename(const std::string& old_path, const std::string& new_path) {
    std::cout << "[rename] starting: " << old_path << " -> " << new_path << std::endl;

    auto it = files_.find(old_path);
    if (it == files_.end()) {
        std::cout << "[rename] failed: source " << old_path << " not found" << std::endl;
        return -1;
    }

    // 如果目标文件已存在，需要先删除它及其所有子内容
    if (files_.find(new_path) != files_.end()) {
        std::cout << "[rename] removing existing target: " << new_path << std::endl;
        remove_recursive(new_path);
    }

    // 如果移动的是目录，需要递归移动所有子内容
    if (it->second.is_directory) {
        std::cout << "[rename] moving directory with recursive content" << std::endl;
        move_recursive(old_path, new_path);
    } else {
        // 移动单个文件
        File new_file = it->second;
        new_file.name = new_path;
        files_[new_path] = new_file;

        // 删除旧文件条目
        files_.erase(it);

        std::cout << "[rename] moved file: " << old_path << " -> " << new_path << std::endl;
    }

    // 更新所有打开的文件描述符
    for (auto& [fd, path] : fd_map_) {
        if (path == old_path || path.find(old_path + "/") == 0) {
            // 完全匹配或者以old_path开头（子文件/子目录）
            std::string new_path_for_fd = path;
            new_path_for_fd.replace(0, old_path.length(), new_path);
            path = new_path_for_fd;
            std::cout << "[rename] updated fd " << fd << ": " << path << std::endl;
        }
    }

    return 0;
}

bool MyFS::is_directory(const std::string& path) {
    auto it = files_.find(path);
    return it != files_.end() && it->second.is_directory;
}

std::vector<std::string> MyFS::list_files(){
    std::vector<std::string> res;
    for (auto& [name, _] : files_)
        res.push_back(name.substr(1)); // 去掉 '/'
    return res;
}

std::vector<std::string> MyFS::list_files_in_dir(const std::string& dir_path) {
    std::vector<std::string> res;
    std::string normalized_dir = normalize_path(dir_path);

    std::cout << "[list_files_in_dir] searching in: " << normalized_dir << std::endl;

    for (auto& [name, file] : files_) {
        std::string normalized_file = normalize_path(name);

        // 检查文件是否直接在指定目录下
        std::string parent = get_parent_dir(normalized_file);
        if (parent == normalized_dir) {
            std::string basename = get_basename(normalized_file);
            res.push_back(basename);
            std::cout << "[list_files_in_dir] found: " << basename << " (full path: " << normalized_file << ")" << std::endl;
        }
    }

    return res;
}

std::string MyFS::normalize_path(const std::string& path) {
    if (path.empty()) return "/";
    if (path != "/" && path.back() == '/') {
        return path.substr(0, path.length() - 1);
    }
    return path;
}

std::string MyFS::get_parent_dir(const std::string& path) {
    std::string normalized = normalize_path(path);

    if (normalized == "/") return "/";

    size_t last_slash = normalized.find_last_of('/');
    if (last_slash == 0) return "/";
    if (last_slash == std::string::npos) return "/";

    return normalized.substr(0, last_slash);
}

std::string MyFS::get_basename(const std::string& path) {
    std::string normalized = normalize_path(path);

    if (normalized == "/") return "";

    size_t last_slash = normalized.find_last_of('/');
    if (last_slash == std::string::npos) return normalized;

    return normalized.substr(last_slash + 1);
}

void MyFS::remove_recursive(const std::string& path) {
    std::cout << "[remove_recursive] removing: " << path << std::endl;

    // 找到所有以path为前缀的文件和目录
    std::vector<std::string> to_remove;

    for (auto const& [file_path, file] : files_) {
        std::string normalized_file_path = normalize_path(file_path);
        std::string normalized_path = normalize_path(path);

        // 如果是完全匹配或者是子路径
        if (normalized_file_path == normalized_path ||
            normalized_file_path.find(normalized_path + "/") == 0) {
            to_remove.push_back(file_path);
        }
    }

    // 删除所有找到的文件和目录
    for (const auto& item_path : to_remove) {
        files_.erase(item_path);
        std::cout << "[remove_recursive] removed: " << item_path << std::endl;
    }
}

void MyFS::move_recursive(const std::string& old_path, const std::string& new_path) {
    std::cout << "[move_recursive] moving: " << old_path << " -> " << new_path << std::endl;

    // 收集所有需要移动的文件
    std::vector<std::pair<std::string, File>> to_move;

    for (auto const& [file_path, file] : files_) {
        std::string normalized_file_path = normalize_path(file_path);
        std::string normalized_old_path = normalize_path(old_path);

        // 如果是完全匹配或者是子路径
        if (normalized_file_path == normalized_old_path ||
            normalized_file_path.find(normalized_old_path + "/") == 0) {
            to_move.push_back({file_path, file});
        }
    }

    // 移动所有文件到新位置
    for (const auto& [old_file_path, file] : to_move) {
        // 计算新路径
        std::string new_file_path = old_file_path;
        new_file_path.replace(0, old_path.length(), new_path);

        // 创建新的文件条目
        File new_file = file;
        new_file.name = new_file_path;
        files_[new_file_path] = new_file;

        // 删除旧文件条目
        files_.erase(old_file_path);

        std::cout << "[move_recursive] moved: " << old_file_path << " -> " << new_file_path << std::endl;
    }
}
