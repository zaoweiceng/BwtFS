#include "core.h"
#include <cstring>
#include <iostream>

using BwtFS::Util::Logger;

int MemoryFS::open(const std::string& path){
    if (files_.find(path) == files_.end()) {
        // 如果文件不存在，则创建为普通文件（保持向后兼容）
        files_[path] = File{path, {}, false};
    }
    int fd = next_fd_++;
    fd_map_[fd] = path;
    // std::cout << "[open] " << path << " -> fd=" << fd << std::endl;
    LOG_DEBUG << "[open] " << path << " -> fd=" << fd;
    return fd;
}

int MemoryFS::create(const std::string& path) {
    // 如果文件不存在，则创建一个空文件
    if (files_.find(path) == files_.end()) {
        files_[path] = File{path, {}, false};  // 普通文件
        std::cout << "[create] " << path << std::endl;
        return 0;
    }
    // 已经存在也返回 0，表示成功（类似 FUSE 语义）
    return 0;
}


int MemoryFS::read(int fd, char* buf, size_t size){
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto& f = files_[it->second];

    size = std::min(size, f.data.size());
    memcpy(buf, f.data.data(), size);
    std::cout << "[read] fd=" << fd << " size=" << size << std::endl;
    return size;
}

int MemoryFS::read(int fd, char* buf, size_t size, off_t offset) {
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


int MemoryFS::write(int fd, const char* buf, size_t size){
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto& f = files_[it->second];

    f.data.assign(buf, buf + size);
    std::cout << "[write] fd=" << fd << " size=" << size << std::endl;
    return size;
}

int MemoryFS::write(int fd, const char* buf, size_t size, off_t offset) {
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


int MemoryFS::remove(const std::string& path){
    std::cout << "[unlink] " << path << std::endl;
    return files_.erase(path) ? 0 : -1;
}

int MemoryFS::close(int fd){
    std::cout << "[close] fd=" << fd << std::endl;
    return fd_map_.erase(fd) ? 0 : -1;
}

int MemoryFS::mkdir(const std::string& path) {
    // 如果目录不存在，则创建一个空目录
    if (files_.find(path) == files_.end()) {
        files_[path] = File{path, {}, true};   // 目录
        std::cout << "[mkdir] " << path << std::endl;
        return 0;
    }
    // 已经存在，如果是目录则返回成功，否则返回错误
    return files_[path].is_directory ? 0 : -1;
}

int MemoryFS::rename(const std::string& old_path, const std::string& new_path) {
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

bool MemoryFS::is_directory(const std::string& path) {
    auto it = files_.find(path);
    return it != files_.end() && it->second.is_directory;
}

std::vector<std::string> MemoryFS::list_files(){
    std::vector<std::string> res;
    for (auto& [name, _] : files_)
        res.push_back(name.substr(1)); // 去掉 '/'
    return res;
}

std::vector<std::string> MemoryFS::list_files_in_dir(const std::string& dir_path) {
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

std::string MemoryFS::normalize_path(const std::string& path) {
    if (path.empty()) return "/";
    if (path != "/" && path.back() == '/') {
        return path.substr(0, path.length() - 1);
    }
    return path;
}

std::string MemoryFS::get_parent_dir(const std::string& path) {
    std::string normalized = normalize_path(path);

    if (normalized == "/") return "/";

    size_t last_slash = normalized.find_last_of('/');
    if (last_slash == 0) return "/";
    if (last_slash == std::string::npos) return "/";

    return normalized.substr(0, last_slash);
}

std::string MemoryFS::get_basename(const std::string& path) {
    std::string normalized = normalize_path(path);

    if (normalized == "/") return "";

    size_t last_slash = normalized.find_last_of('/');
    if (last_slash == std::string::npos) return normalized;

    return normalized.substr(last_slash + 1);
}

void MemoryFS::remove_recursive(const std::string& path) {
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

void MemoryFS::move_recursive(const std::string& old_path, const std::string& new_path) {
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

int BwtFSMounter::open(const std::string& path){
    if (path_fd_map_.find(path) != path_fd_map_.end()) {
        // 文件已经打开，返回已有的文件描述符
        int existing_fd = path_fd_map_[path];
        LOG_DEBUG << "[open] " << path << " already opened -> fd=" << existing_fd;
        return existing_fd;
    }
    // 创建新的 bw_tree 对象并存储
    std::string file_token = file_manager_.getFileToken(path);
    if (file_token.empty() || file_token == "") {
        LOG_ERROR << "文件不存在: " << path;
        return -1;
    }
    BwtFS::Node::bw_tree* tree = new BwtFS::Node::bw_tree(file_token, false);
    int fd = next_fd_++;
    fd_map_[fd] = path;
    path_fd_map_[path] = fd;
    fd_tree_map_[fd] = tree;
    LOG_DEBUG << "[open] " << path << " -> fd=" << fd;
    return fd;
}

int BwtFSMounter::create(const std::string& path) {
    // 创建文件
    int fd = next_fd_++;
    fd_map_[fd] = path;
    path_fd_map_[path] = fd;
    // 在文件管理器中添加文件
    fd_write_wait_map_[fd] = new BwtFS::Node::bw_tree(); // 创建一个空的 bw_tree 对象用于写入
    fd_file_size_map_[fd] = 0; // 初始化文件大小为0
    LOG_DEBUG << "[create] " << path;
    return 0;
}

int BwtFSMounter::read(int fd, char* buf, size_t size){
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto tree_it = fd_tree_map_.find(fd);
    if (tree_it == fd_tree_map_.end()) return -1;
    BwtFS::Node::bw_tree* tree = tree_it->second;
    // tree->read
    // Binary read(size_t index, size_t size)
    Binary data = tree->read(0, size); // 从偏移0开始读取
    size_t bytes_read = std::min(size, data.size());
    memcpy(buf, data.data(), bytes_read);
    LOG_DEBUG << "[read] fd=" << fd << " size=" << bytes_read;
    return bytes_read;
}

int BwtFSMounter::read(int fd, char* buf, size_t size, off_t offset) {
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto tree_it = fd_tree_map_.find(fd);
    if (tree_it == fd_tree_map_.end()) return -1;
    BwtFS::Node::bw_tree* tree = tree_it->second;
    // tree->read
    // Binary read(size_t index, size_t size)
    Binary data = tree->read(offset, size); // 从指定偏移开始读取
    size_t bytes_read = std::min(size, data.size());
    memcpy(buf, data.data(), bytes_read);
    LOG_DEBUG << "[read] fd=" << fd << " offset=" << offset << " size=" << bytes_read;
    return bytes_read;
}

int BwtFSMounter::write(int fd, const char* buf, size_t size){
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto tree_it = fd_write_wait_map_.find(fd);
    if (tree_it == fd_write_wait_map_.end()) return -1;
    BwtFS::Node::bw_tree* tree = tree_it->second;
    // tree->write
    tree->write(const_cast<char*>(buf), size);
    LOG_DEBUG << "[write] fd=" << fd << " size=" << size;
    fd_file_size_map_[fd] += size;
    return size;
}

int BwtFSMounter::write(int fd, const char* buf, size_t size, off_t offset) {
    return write(fd, buf, size);
}

int BwtFSMounter::remove(const std::string& path){
    LOG_DEBUG << "[unlink] " << path;
    std::string file_token = file_manager_.getFileToken(path);
    if (file_token.empty() || file_token == "") {
        LOG_ERROR << "文件不存在: " << path;
        return -1;
    }
    BwtFS::Node::bw_tree tree(file_token, false);
    tree.delete_file();
    file_manager_.remove(path);
    return 0;
}

int BwtFSMounter::close(int fd){
    LOG_DEBUG << "[close] fd=" << fd;
    auto it = fd_tree_map_.find(fd);
    if (it != fd_tree_map_.end()) {
        delete it->second;
        fd_tree_map_.erase(it);
    }
    auto write_it = fd_write_wait_map_.find(fd);
    if (write_it != fd_write_wait_map_.end()) {
        // 完成写入并保存文件
        write_it->second->flush();
        write_it->second->join();
        std::string file_token = write_it->second->get_token();
        // 在文件管理器中注册新文件
        size_t file_size = fd_file_size_map_[fd];
        file_manager_.addFile(fd_map_[fd], file_token, file_size);
        delete write_it->second;
        fd_write_wait_map_.erase(write_it);
        fd_file_size_map_.erase(fd);
    }
    auto path_it = fd_map_.find(fd);
    if (path_it != fd_map_.end()) {
        path_fd_map_.erase(path_it->second);
        fd_map_.erase(path_it);
    }
    return 0;
}

int BwtFSMounter::mkdir(const std::string& path) {
    LOG_DEBUG << "[mkdir] " << path;
    file_manager_.createDir(path);
    return 0;
}

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

int BwtFSMounter::rename(const std::string& old_path, const std::string& new_path) {
    LOG_DEBUG << "[rename] " << old_path << " -> " << new_path;
    // file_manager_.rename(old_path, new_path);
    // 根据oldpath和newpath判断是重命名文件、文件夹，还是移动
    std::vector<std::string> old_paths = splitPath(old_path);
    std::vector<std::string> new_paths = splitPath(new_path);
    if (old_paths.size() == new_paths.size()) {
        // 重命名
        bool is_rename = true;
        for (size_t i = 0; i < old_paths.size() - 1; i++) {
            if (old_paths[i] != new_paths[i]) {
                is_rename = false;
                break;
            }
        }
        if (is_rename) {
            file_manager_.rename(old_path, new_path);
            LOG_DEBUG << "[rename] Detected as rename operation.";
            return 0;
        } else {
            file_manager_.move(old_path, new_path);
            LOG_DEBUG << "[rename] Detected as move operation.";
            return 0;
        }
    } else {
        // 移动
        file_manager_.move(old_path, new_path);
    }
    return 0;
}

bool BwtFSMounter::is_directory(const std::string& path) {
    auto file_node = file_manager_.getFile(path);
    return file_node.is_dir;
}

std::vector<std::string> BwtFSMounter::list_files(){
    auto file_nodes = file_manager_.listDir("/");
    std::vector<std::string> res;
    for (auto& node : file_nodes) {
        res.push_back(node.name);
    }
    return res;
}

std::vector<std::string> BwtFSMounter::list_files_in_dir(const std::string& dir_path) {
    auto file_nodes = file_manager_.listDir(dir_path);
    std::vector<std::string> res;
    for (auto& node : file_nodes) {
        res.push_back(node.name);
    }
    return res;
}

std::string BwtFSMounter::normalize_path(const std::string& path) {
    if (path.empty()) return "/";
    if (path != "/" && path.back() == '/') {
        return path.substr(0, path.length() - 1);
    }
    return path;
}

std::string BwtFSMounter::get_parent_dir(const std::string& path) {
    std::string normalized = normalize_path(path);

    if (normalized == "/") return "/";

    size_t last_slash = normalized.find_last_of('/');
    if (last_slash == 0) return "/";
    if (last_slash == std::string::npos) return "/";

    return normalized.substr(0, last_slash);
}

std::string BwtFSMounter::get_basename(const std::string& path) {
    std::string normalized = normalize_path(path);

    if (normalized == "/") return "";

    size_t last_slash = normalized.find_last_of('/');
    if (last_slash == std::string::npos) return normalized;

    return normalized.substr(last_slash + 1);
}

void BwtFSMounter::remove_recursive(const std::string& path) {
    LOG_DEBUG << "[remove_recursive] " << path;
    file_manager_.remove(path);
}

void BwtFSMounter::move_recursive(const std::string& old_path, const std::string& new_path) {
    LOG_DEBUG << "[move_recursive] " << old_path << " -> " << new_path;
    file_manager_.move(old_path, new_path);
}

bool BwtFSMounter::file_exists(const std::string& path) {
    auto file_node = file_manager_.getFile(path);
    return !file_node.name.empty();
}

FileNode BwtFSMounter::getFileNode(const std::string& path) {
    auto file_node = file_manager_.getFile(path);
    return file_node;
}

SystemInfo BwtFSMounter::getSystemInfo() {
    return system_manager_.getSystemInfo();
}



