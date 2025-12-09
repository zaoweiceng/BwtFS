#include "core.h"
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <set>

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
        LOG_DEBUG << "[create] " << path;
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
    LOG_DEBUG << "[read] fd=" << fd << " size=" << size;
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
    LOG_DEBUG << "[read] fd=" << fd << " offset=" << offset << " size=" << size;
    return size;
}

int MemoryFS::write(int fd, const char* buf, size_t size){
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto& f = files_[it->second];

    f.data.assign(buf, buf + size);
    LOG_DEBUG << "[write] fd=" << fd << " size=" << size;
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
    LOG_DEBUG << "[write] fd=" << fd << " offset=" << offset << " size=" << size;
    return size;
}

int MemoryFS::remove(const std::string& path){
    LOG_DEBUG << "[unlink] " << path;
    return files_.erase(path) ? 0 : -1;
}

int MemoryFS::close(int fd){
    LOG_DEBUG << "[close] fd=" << fd;
    return fd_map_.erase(fd) ? 0 : -1;
}

int MemoryFS::mkdir(const std::string& path) {
    // 如果目录不存在，则创建一个空目录
    if (files_.find(path) == files_.end()) {
        files_[path] = File{path, {}, true};   // 目录
        LOG_DEBUG << "[mkdir] " << path;
        return 0;
    }
    // 已经存在，如果是目录则返回成功，否则返回错误
    return files_[path].is_directory ? 0 : -1;
}

int MemoryFS::rename(const std::string& old_path, const std::string& new_path) {
    LOG_DEBUG << "[rename] starting: " << old_path << " -> " << new_path;

    auto it = files_.find(old_path);
    if (it == files_.end()) {
        LOG_DEBUG << "[rename] failed: source " << old_path << " not found";
        return -1;
    }

    // 如果目标文件已存在，需要先删除它及其所有子内容
    if (files_.find(new_path) != files_.end()) {
        LOG_DEBUG << "[rename] removing existing target: " << new_path;
        remove_recursive(new_path);
    }

    // 如果移动的是目录，需要递归移动所有子内容
    if (it->second.is_directory) {
        LOG_DEBUG << "[rename] moving directory with recursive content";
        move_recursive(old_path, new_path);
    } else {
        // 移动单个文件
        File new_file = it->second;
        new_file.name = new_path;
        files_[new_path] = new_file;

        // 删除旧文件条目
        files_.erase(it);

        LOG_DEBUG << "[rename] moved file: " << old_path << " -> " << new_path;
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
    LOG_DEBUG << "[is_directory] checking: " << path;
    auto it = files_.find(path);
    return it != files_.end() && it->second.is_directory;
}

std::vector<std::string> MemoryFS::list_files(){
    LOG_DEBUG << "[list_files] listing all files";
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
    LOG_DEBUG << "[open] requested path: " << path;

    // 确保路径以/开头
    std::string normalized_path = path;
    if (!path.empty() && path[0] != '/') {
        normalized_path = "/" + path;
    }

    if (path_fd_map_.find(normalized_path) != path_fd_map_.end()) {
        // 文件已经打开，返回已有的文件描述符
        int existing_fd = path_fd_map_[normalized_path];
        LOG_DEBUG << "[open] " << normalized_path << " already opened -> fd=" << existing_fd;
        return existing_fd;
    }

    // 检查文件是否存在
    std::string file_token = file_manager_.getFileToken(normalized_path);
    if (file_token.empty()) {
        LOG_ERROR << "文件不存在或正在写入中: " << normalized_path;
        return -1;
    }

    // 创建新的 bw_tree 对象并存储
    try {
        BwtFS::Node::bw_tree* tree = new BwtFS::Node::bw_tree(file_token, false);
        int fd = next_fd_++;
        fd_map_[fd] = normalized_path;
        path_fd_map_[normalized_path] = fd;
        fd_tree_map_[fd] = tree;
        LOG_DEBUG << "[open] " << normalized_path << " -> fd=" << fd;
        return fd;
    } catch (const std::exception& e) {
        LOG_ERROR << "[open] failed to create bw_tree: " << e.what();
        return -1;
    }
}

int BwtFSMounter::create(const std::string& path) {
    LOG_DEBUG << "[create] requested path: " << path;

    // 确保路径以/开头
    std::string normalized_path = path;
    if (!path.empty() && path[0] != '/') {
        normalized_path = "/" + path;
    }

    // 创建文件描述符
    int fd = next_fd_++;
    fd_map_[fd] = normalized_path;
    path_fd_map_[normalized_path] = fd;

    // 检查是否是系统临时文件
    if (isSystemTempFile(normalized_path)) {
        LOG_DEBUG << "[create] using memory_fs for system temp file: " << normalized_path;

        // 在memory_fs中创建文件
        int memory_fd = memory_fs_.create(normalized_path);
        if (memory_fd < 0) {
            LOG_ERROR << "[create] failed to create in memory_fs: " << normalized_path;
            fd_map_.erase(fd);
            path_fd_map_.erase(normalized_path);
            return -EIO;
        }

        // 将memory_fd映射到我们的fd，但标记为内存文件
        fd_tree_map_[fd] = nullptr; // nullptr表示这是内存文件
        LOG_DEBUG << "[create] created in memory_fs: " << normalized_path << " -> fd=" << fd;

    } else {
        // 普通用户文件，使用BwtFS
        LOG_DEBUG << "[create] using BwtFS for user file: " << normalized_path;

        // 创建一个空的 bw_tree 对象用于写入（按照CMD中的正确方式）
        try {
            fd_write_wait_map_[fd] = new BwtFS::Node::bw_tree();
            LOG_DEBUG << "[create] created bw_tree for fd=" << fd;
        } catch (const std::exception& e) {
            LOG_ERROR << "[create] failed to create bw_tree: " << e.what();
            fd_write_wait_map_.erase(fd);
            return -EIO;
        }

        fd_file_size_map_[fd] = 0;

        // 在文件管理器中创建文件占位符（没有token，表示文件正在创建中）
        file_manager_.addFile(normalized_path, "PENDING", 0);
        LOG_DEBUG << "[create] created in BwtFS: " << normalized_path << " -> fd=" << fd;
    }
    return 0;
}

int BwtFSMounter::read(int fd, char* buf, size_t size){
    LOG_DEBUG << "[read] requested fd=" << fd << " size=" << size;
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto tree_it = fd_tree_map_.find(fd);
    if (tree_it == fd_tree_map_.end()) return -1;

    // 检查是否是内存文件（nullptr表示内存文件）
    if (tree_it->second == nullptr) {
        // 从memory_fs读取
        std::string path = it->second;
        auto memory_file_it = memory_fs_.files_.find(path);
        if (memory_file_it == memory_fs_.files_.end()) {
            LOG_DEBUG << "[read] memory file not found: " << path;
            return -1;
        }

        const auto& data = memory_file_it->second.data;
        size_t bytes_read = std::min(size, data.size());
        memcpy(buf, data.data(), bytes_read);
        LOG_DEBUG << "[read] from memory_fs fd=" << fd << " size=" << bytes_read;
        return bytes_read;

    } else {
        // 从BwtFS读取
        BwtFS::Node::bw_tree* tree = tree_it->second;
        Binary data = tree->read(0, size); // 从偏移0开始读取
        size_t bytes_read = std::min(size, data.size());
        memcpy(buf, data.data(), bytes_read);
        LOG_DEBUG << "[read] from BwtFS fd=" << fd << " size=" << bytes_read;
        return bytes_read;
    }
}

int BwtFSMounter::read(int fd, char* buf, size_t size, off_t offset) {
    LOG_DEBUG << "[read] requested fd=" << fd << " offset=" << offset << " size=" << size;
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;
    auto tree_it = fd_tree_map_.find(fd);
    if (tree_it == fd_tree_map_.end()) return -1;

    // 检查是否是内存文件（nullptr表示内存文件）
    if (tree_it->second == nullptr) {
        // 从memory_fs读取
        std::string path = it->second;
        auto memory_file_it = memory_fs_.files_.find(path);
        if (memory_file_it == memory_fs_.files_.end()) {
            LOG_DEBUG << "[read] memory file not found: " << path;
            return -1;
        }

        const auto& data = memory_file_it->second.data;
        if (offset >= data.size()) {
            return 0; // 超出文件范围
        }

        size_t available_bytes = data.size() - offset;
        size_t bytes_read = std::min(size, available_bytes);
        memcpy(buf, data.data() + offset, bytes_read);
        LOG_DEBUG << "[read] from memory_fs fd=" << fd << " offset=" << offset << " size=" << bytes_read;
        return bytes_read;

    } else {
        // 从BwtFS读取
        BwtFS::Node::bw_tree* tree = tree_it->second;
        Binary data = tree->read(offset, size); // 从指定偏移开始读取
        size_t bytes_read = std::min(size, data.size());
        memcpy(buf, data.data(), bytes_read);
        LOG_DEBUG << "[read] from BwtFS fd=" << fd << " offset=" << offset << " size=" << bytes_read;
        return bytes_read;
    }
}

int BwtFSMounter::write(int fd, const char* buf, size_t size){
    LOG_DEBUG << "[write] requested fd=" << fd << " size=" << size;
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) {
        LOG_ERROR << "[write] Invalid fd map entry for fd=" << fd;
        return -EBADF;
    }

    auto tree_it = fd_write_wait_map_.find(fd);
    if (tree_it == fd_write_wait_map_.end()) {
        LOG_ERROR << "[write] No bw_tree found for fd=" << fd;
        return -EBADF;
    }

    BwtFS::Node::bw_tree* tree = tree_it->second;
    if (!tree) {
        LOG_ERROR << "[write] bw_tree is null for fd=" << fd;
        return -EIO;
    }

    try {
        // 按照CMD中的正确方式写入数据
        tree->write(const_cast<char*>(buf), size);
        fd_file_size_map_[fd] += size;
        LOG_DEBUG << "[write] successfully wrote " << size << " bytes to fd=" << fd << ", total: " << fd_file_size_map_[fd];
        return size;
    } catch (const std::exception& e) {
        LOG_ERROR << "[write] Error writing to fd=" << fd << ": " << e.what();
        return -EIO;
    }
}

int BwtFSMounter::write(int fd, const char* buf, size_t size, off_t offset) {
    LOG_DEBUG << "[write] requested fd=" << fd << " offset=" << offset << " size=" << size;
    // 目前不支持偏移写入，直接调用不带偏移的写入函数
    return write(fd, buf, size);
}

int BwtFSMounter::remove(const std::string& path){
    LOG_DEBUG << "[unlink] " << path;
    std::string file_token = file_manager_.getFileToken(path);
    if (file_token.empty() || file_token == "") {
        LOG_ERROR << "文件不存在: " << path;
        return -1;
    }
    BwtFS::Node::bw_tree tree(file_token, true);
    tree.delete_file();
    file_manager_.remove(path);
    return 0;
}

int BwtFSMounter::close(int fd){
    LOG_DEBUG << "[close] fd=" << fd;

    std::string file_path;
    auto path_it = fd_map_.find(fd);
    if (path_it != fd_map_.end()) {
        file_path = path_it->second;
    }

    // 处理只读打开的文件
    auto it = fd_tree_map_.find(fd);
    if (it != fd_tree_map_.end()) {
        delete it->second;
        fd_tree_map_.erase(it);
    }

    // 处理写入的文件 - 采用后台异步处理方式
    auto write_it = fd_write_wait_map_.find(fd);
    if (write_it != fd_write_wait_map_.end()) {
        if (write_it->second) {
            LOG_DEBUG << "[close] starting async finalize for fd=" << fd;

            // 将bw_tree对象移交给后台线程处理，立即返回
            BwtFS::Node::bw_tree* tree_to_finalize = write_it->second;
            size_t file_size = fd_file_size_map_[fd];

            // 从映射中移除，避免在FUSE回调中持有资源
            fd_write_wait_map_.erase(write_it);
            fd_file_size_map_.erase(fd);

            // 启动后台线程处理flush/join（避免在FUSE回调中阻塞）
            std::thread([this, tree_to_finalize, file_path, file_size]() {
                LOG_DEBUG << "[async] background finalize started for " << file_path;

                try {
                    // 按照net项目中的正确顺序处理
                    LOG_DEBUG << "[async] flushing for " << file_path;
                    tree_to_finalize->flush();

                    LOG_DEBUG << "[async] joining for " << file_path;
                    tree_to_finalize->join();

                    LOG_DEBUG << "[async] getting token for " << file_path;
                    std::string file_token = tree_to_finalize->get_token();

                    LOG_DEBUG << "[async] got token '" << file_token << "' for " << file_path;

                    if (!file_token.empty() && file_token != "PENDING") {
                        // 成功获取token，更新文件记录
                        LOG_DEBUG << "[async] updating file record for " << file_path;
                        file_manager_.remove(file_path);
                        file_manager_.addFile(file_path, file_token, file_size);
                        LOG_INFO << "[async] successfully finalized " << file_path << " with token " << file_token;
                    } else {
                        // 失败，移除PENDING记录
                        LOG_WARNING << "[async] finalize failed for " << file_path;
                        file_manager_.remove(file_path);
                    }

                } catch (const std::exception& e) {
                    LOG_ERROR << "[async] exception during finalize for " << file_path << ": " << e.what();
                    // 清理失败的文件记录
                    file_manager_.remove(file_path);
                } catch (...) {
                    LOG_ERROR << "[async] unknown exception during finalize for " << file_path;
                    file_manager_.remove(file_path);
                }

                // 清理bw_tree对象（按照net项目的做法）
                delete tree_to_finalize;
                LOG_DEBUG << "[async] background finalize completed for " << file_path;

            }).detach();

            LOG_DEBUG << "[close] async finalize initiated for fd=" << fd;
        } else {
            LOG_ERROR << "[close] bw_tree is null for fd=" << fd;
            fd_write_wait_map_.erase(write_it);
            fd_file_size_map_.erase(fd);
        }
    }

    // 清理路径映射
    if (path_it != fd_map_.end()) {
        path_fd_map_.erase(path_it->second);
        fd_map_.erase(path_it);
    }

    LOG_DEBUG << "[close] completed fd=" << fd;
    return 0;
}

int BwtFSMounter::mkdir(const std::string& path) {
    LOG_DEBUG << "[mkdir] " << path;

    // 确保路径以/开头
    std::string normalized_path = path;
    if (!path.empty() && path[0] != '/') {
        normalized_path = "/" + path;
    }

    // 确保文件管理器已初始化
    if (!file_manager_.createDir(normalized_path)) {
        LOG_ERROR << "Failed to create directory: " << normalized_path;
        return -EIO;
    }

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
            // 这是重命名操作，只需要传递文件名部分
            std::string old_basename = get_basename(old_path);
            std::string new_basename = get_basename(new_path);
            file_manager_.rename(old_path, new_basename);
            LOG_DEBUG << "[rename] Detected as rename operation: " << old_path << " -> " << new_basename;
            return 0;
        } else {
            file_manager_.move(old_path, new_path);
            LOG_DEBUG << "[rename] Detected as move operation: " << old_path << " -> " << new_path;
            return 0;
        }
    } else {
        // 移动
        file_manager_.move(old_path, new_path);
    }
    return 0;
}

bool BwtFSMounter::is_directory(const std::string& path) {
    LOG_DEBUG << "[is_directory] checking: " << path;

    // 根目录总是目录
    if (path == "/" || path.empty()) {
        return true;
    }

    // 确保路径以/开头
    std::string normalized_path = path;
    if (!path.empty() && path[0] != '/') {
        normalized_path = "/" + path;
    }

    auto file_node = file_manager_.getFile(normalized_path);
    // 如果文件不存在，返回false（这可能意味着路径不存在或不是目录）
    return !file_node.name.empty() && file_node.is_dir;
}

std::vector<std::string> BwtFSMounter::list_files(){
    LOG_DEBUG << "[list_files] listing all files";
    auto file_nodes = file_manager_.listDir("/");
    std::vector<std::string> res;
    for (auto& node : file_nodes) {
        res.push_back(node.name);
    }
    return res;
}

std::vector<std::string> BwtFSMounter::list_files_in_dir(const std::string& dir_path) {
    // LOG_DEBUG << "[list_files_in_dir] listing files in dir: " << dir_path;
    std::vector<std::string> res;
    std::set<std::string> unique_names; // 用于避免重复文件名

    // 1. 从BwtFS获取普通用户文件
    auto file_nodes = file_manager_.listDir(dir_path);
    for (auto& node : file_nodes) {
        if (unique_names.insert(node.name).second) {
            res.push_back(node.name);
            // LOG_DEBUG << "[list_files_in_dir] found in BwtFS: " << node.name;
        }
    }

    // 2. 从memory_fs获取系统临时文件
    std::string normalized_dir = dir_path;
    if (normalized_dir.empty()) normalized_dir = "/";
    if (normalized_dir != "/" && normalized_dir.back() == '/') {
        normalized_dir = normalized_dir.substr(0, normalized_dir.length() - 1);
    }

    for (auto& [name, file] : memory_fs_.files_) {
        std::string normalized_file = name;
        if (normalized_file != "/" && normalized_file.back() == '/') {
            normalized_file = normalized_file.substr(0, normalized_file.length() - 1);
        }

        // 检查文件是否直接在指定目录下
        std::string parent = memory_fs_.get_parent_dir(normalized_file);
        if (parent == normalized_dir) {
            std::string basename = memory_fs_.get_basename(normalized_file);
            // 只添加系统临时文件，避免重复
            if (isSystemTempFile(normalized_file) && unique_names.insert(basename).second) {
                res.push_back(basename);
                LOG_DEBUG << "[list_files_in_dir] found in memory_fs: " << basename << " (system temp file)";
            }
        }
    }

    return res;
}

std::string BwtFSMounter::normalize_path(const std::string& path) {
    LOG_DEBUG << "[normalize_path] normalizing: " << path;
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
    LOG_DEBUG << "[file_exists] checking: " << path;
    auto file_node = file_manager_.getFile(path);
    return !file_node.name.empty();
}

FileNode BwtFSMounter::getFileNode(const std::string& path) {
    // 确保路径以/开头
    std::string normalized_path = path;
    if (!path.empty() && path[0] != '/') {
        normalized_path = "/" + path;
    }

    // 首先检查是否是系统临时文件，如果是则从memory_fs获取
    if (isSystemTempFile(normalized_path)) {
        LOG_DEBUG << "[getFileNode] getting from memory_fs: " << normalized_path;

        // 检查memory_fs中是否存在该文件
        auto it = memory_fs_.files_.find(normalized_path);
        if (it != memory_fs_.files_.end()) {
            // 在memory_fs中找到文件，返回对应的FileNode
            FileNode node;
            node.name = it->second.name;
            node.token = "memory_fs"; // 标识为内存文件
            node.file_size = it->second.data.size();
            node.is_dir = it->second.is_directory;
            LOG_DEBUG << "[getFileNode] found in memory_fs: " << normalized_path;
            return node;
        } else {
            // memory_fs中也不存在，返回空节点
            LOG_DEBUG << "[getFileNode] not found in memory_fs: " << normalized_path;
            return FileNode();
        }
    }

    // 普通用户文件，从BwtFS获取
    auto file_node = file_manager_.getFile(normalized_path);
    return file_node;
}

SystemInfo BwtFSMounter::getSystemInfo() {
    return system_manager_.getSystemInfo();
}



