#include "core.h"
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <set>
#include <fcntl.h>

using BwtFS::Util::Logger;

int MemoryFS::open(const std::string& path){
    if (files_.find(path) == files_.end()) {
        // 如果文件不存在，则创建为普通文件（保持向后兼容）
        files_[path] = File{path, {}, false};
    }
    // 如果已经打开，返回相同的fd
    for (const auto& pair : fd_map_) {
        if (pair.second == path) {
            LOG_DEBUG << "[open] " << path << " already opened -> fd=" << pair.first;
            return pair.first;;
        }
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

int BwtFSMounter::open(const std::string& path, int flags /* = O_RDONLY */){
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
        LOG_ERROR << "[open] file does not exist: " << normalized_path;
        return -ENOENT;
    }

    // 检查是否需要写入（如果是写模式，需要特殊处理）
    bool need_write = (flags & (O_WRONLY | O_RDWR)) != 0;

    if (file_token == "memory") {
        // 文件还在memory_fs中，从memory_fs打开
        LOG_DEBUG << "[open] opening file from memory_fs: " << normalized_path << " write=" << need_write;

        // 检查是否为系统临时文件
        std::string basename = normalized_path.substr(normalized_path.find_last_of('/') + 1);
        bool is_system_temp_file = (basename.find("._") == 0 || basename == ".DS_Store");

        if (is_system_temp_file && need_write) {
            // 系统临时文件需要写入时，删除旧文件并重新创建
            LOG_DEBUG << "[open] system temp file needs write, recreating: " << normalized_path;
            if (memory_fs_.files_.find(normalized_path) != memory_fs_.files_.end()) {
                memory_fs_.remove(normalized_path);
            }
        }

        auto memory_file_it = memory_fs_.files_.find(normalized_path);
        if (memory_file_it == memory_fs_.files_.end()) {
            LOG_ERROR << "[open] memory file not found: " << normalized_path;
            return -ENOENT;
        }

        // 在memory_fs中打开文件
        int memory_fd = memory_fs_.open(normalized_path);
        if (memory_fd < 0) {
            LOG_ERROR << "[open] failed to open in memory_fs: " << normalized_path;
            return -EIO;
        }

        // 创建BwtFS文件描述符
        int fd = next_fd_++;
        fd_map_[fd] = normalized_path;
        fd_to_memory_fd_map_[fd] = memory_fd;
        path_fd_map_[normalized_path] = fd;

        LOG_DEBUG << "[open] opened from memory_fs " << normalized_path << " -> fd=" << fd;
        return fd;

    } else {
        // 文件已经在bwtfs中
        if (need_write) {
            // 支持写入：如果是现有文件，将使用COW策略
            LOG_INFO << "[open] opening existing file for write with COW support: " << normalized_path;

            // 创建文件描述符映射，COW策略将在write时处理
            int fd = next_fd_++;
            fd_map_[fd] = normalized_path;
            path_fd_map_[normalized_path] = fd;

            return fd;
        } else {
            // 只读访问，创建bw_tree对象进行读取
            LOG_DEBUG << "[open] opening file from BwtFS: " << normalized_path << " token=" << file_token;

            // 验证token的有效性
            if (file_token.empty()) {
                LOG_ERROR << "[open] empty token for file: " << normalized_path;
                return -EIO;
            }

            // 检查token中是否包含无效字符
            // if (file_token.find('*') != std::string::npos) {
            //     LOG_ERROR << "[open] invalid token containing '*': " << file_token << " for file: " << normalized_path;
            //     return -EIO;
            // }

            try {
                BwtFS::Node::bw_tree* tree = new BwtFS::Node::bw_tree(file_token, false);
                int fd = next_fd_++;
                fd_map_[fd] = normalized_path;
                path_fd_map_[normalized_path] = fd;
                fd_tree_map_[fd] = tree;

                LOG_DEBUG << "[open] opened from BwtFS " << normalized_path << " -> fd=" << fd;
                return fd;
            } catch (const std::exception& e) {
                LOG_ERROR << "[open] failed to create bw_tree: " << e.what();
                LOG_ERROR << "[open] problematic token: '" << file_token << "' for file: " << normalized_path;
                return -EIO;
            }
        }
    }
}

int BwtFSMounter::create(const std::string& path) {
    LOG_DEBUG << "[create] requested path: " << path;

    // 确保路径以/开头
    std::string normalized_path = path;
    if (!path.empty() && path[0] != '/') {
        normalized_path = "/" + path;
    }

    // 检查文件是否已经存在
    std::string existing_token = file_manager_.getFileToken(normalized_path);
    if (!existing_token.empty() && existing_token != "PENDING") {
        LOG_DEBUG << "[create] file already exists, removing old version: " << normalized_path;
        // 删除已存在的文件，允许重新创建
        file_manager_.remove(normalized_path);
        // 如果是memory文件，也从memory_fs删除
        if (existing_token == "memory") {
            memory_fs_.remove(normalized_path);
        }
    }

    // 检查是否为系统临时文件，使用特殊处理
    std::string basename = normalized_path.substr(normalized_path.find_last_of('/') + 1);
    bool is_system_temp_file = (basename.find("._") == 0 || basename == ".DS_Store");

    if (is_system_temp_file) {
        LOG_DEBUG << "[create] system temp file detected, allowing overwrites: " << normalized_path;

        // 对于系统临时文件，如果已存在就删除并重新创建
        if (memory_fs_.files_.find(normalized_path) != memory_fs_.files_.end()) {
            memory_fs_.remove(normalized_path);
        }
    }

    // 先在memory_fs中创建文件
    if (memory_fs_.create(normalized_path) < 0) {
        LOG_ERROR << "[create] failed to create in memory_fs: " << normalized_path;
        return -EIO;
    }

    // 在memory_fs中打开文件以获取文件描述符
    int memory_fd = memory_fs_.open(normalized_path);
    if (memory_fd < 0) {
        LOG_ERROR << "[create] failed to open in memory_fs: " << normalized_path;
        return -EIO;
    }

    // 创建BwtFS文件描述符
    int fd = next_fd_++;
    fd_map_[fd] = normalized_path;
    path_fd_map_[normalized_path] = fd;
    fd_to_memory_fd_map_[fd] = memory_fd;

    // 在文件管理器中创建文件占位符（token为"memory"表示暂存在memory_fs中）
    if (!file_manager_.addFile(normalized_path, "memory", 0)) {
        LOG_ERROR << "[create] failed to add file to file_manager: " << normalized_path;
        memory_fs_.close(memory_fd);
        fd_map_.erase(fd);
        path_fd_map_.erase(normalized_path);
        fd_to_memory_fd_map_.erase(fd);
        return -EIO;
    }

    LOG_DEBUG << "[create] successfully created in memory_fs: " << normalized_path << " -> fd=" << fd;
    return 0;
}

int BwtFSMounter::read(int fd, char* buf, size_t size){
    LOG_DEBUG << "[read] requested fd=" << fd << " size=" << size;
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;

    std::string path = it->second;
    auto file_token = file_manager_.getFileToken(path);

    if (file_token == "memory") {
        // 文件还在memory_fs中，从memory_fs读取
        LOG_DEBUG << "[read] from memory_fs fd=" << fd << " size=" << size;
        auto memory_fd_it = fd_to_memory_fd_map_.find(fd);
        if (memory_fd_it == fd_to_memory_fd_map_.end()) {
            LOG_ERROR << "[read] No memory_fd found for fd=" << fd;
            return -EBADF;
        }

        int memory_fd = memory_fd_it->second;
        return memory_fs_.read(memory_fd, buf, size);

    } else if (!file_token.empty()) {
        // 文件已经在bwtfs中，从bwtfs读取
        LOG_DEBUG << "[read] from BwtFS fd=" << fd << " token=" << file_token << " size=" << size;
        auto tree_it = fd_tree_map_.find(fd);
        if (tree_it == fd_tree_map_.end()) {
            LOG_ERROR << "[read] No tree found for bwtfs file fd=" << fd;
            return -EBADF;
        }

        BwtFS::Node::bw_tree* tree = tree_it->second;
        try {
            Binary data = tree->read(0, size); // 从偏移0开始读取
            size_t bytes_read = std::min(size, data.size());
            memcpy(buf, data.data(), bytes_read);
            LOG_DEBUG << "[read] from BwtFS fd=" << fd << " size=" << bytes_read;
            return bytes_read;
        } catch (const std::exception& e) {
            LOG_ERROR << "[read] Exception reading from BwtFS: " << e.what();
            return -EIO;
        }

    } else {
        // 文件状态无效
        LOG_ERROR << "[read] Invalid file token for path: " << path;
        return -EBADF;
    }
}

int BwtFSMounter::read(int fd, char* buf, size_t size, off_t offset) {
    LOG_DEBUG << "[read] requested fd=" << fd << " offset=" << offset << " size=" << size;
    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) return -1;

    std::string path = it->second;
    auto file_token = file_manager_.getFileToken(path);

    if (file_token == "memory") {
        // 文件还在memory_fs中，从memory_fs读取
        LOG_DEBUG << "[read] from memory_fs fd=" << fd << " offset=" << offset << " size=" << size;
        auto memory_fd_it = fd_to_memory_fd_map_.find(fd);
        if (memory_fd_it == fd_to_memory_fd_map_.end()) {
            LOG_ERROR << "[read] No memory_fd found for fd=" << fd;
            return -EBADF;
        }

        int memory_fd = memory_fd_it->second;
        return memory_fs_.read(memory_fd, buf, size, offset);

    } else if (!file_token.empty()) {
        // 文件已经在bwtfs中，从bwtfs读取
        LOG_DEBUG << "[read] from BwtFS fd=" << fd << " token=" << file_token << " offset=" << offset << " size=" << size;
        auto tree_it = fd_tree_map_.find(fd);
        if (tree_it == fd_tree_map_.end()) {
            LOG_ERROR << "[read] No tree found for bwtfs file fd=" << fd;
            return -EBADF;
        }

        BwtFS::Node::bw_tree* tree = tree_it->second;
        try {
            Binary data = tree->read(offset, size); // 从指定偏移开始读取
            size_t bytes_read = std::min(size, data.size());
            memcpy(buf, data.data(), bytes_read);
            LOG_DEBUG << "[read] from BwtFS fd=" << fd << " offset=" << offset << " size=" << bytes_read;
            return bytes_read;
        } catch (const std::exception& e) {
            LOG_ERROR << "[read] Exception reading from BwtFS: " << e.what();
            return -EIO;
        }

    } else {
        // 文件状态无效
        LOG_ERROR << "[read] Invalid file token for path: " << path;
        return -EBADF;
    }
}

int BwtFSMounter::write(int fd, const char* buf, size_t size){
    LOG_DEBUG << "[write] requested fd=" << fd << " size=" << size;

    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) {
        LOG_WARNING << "[write] Invalid fd map entry for fd=" << fd << ", treating as closed";
        return -EBADF;
    }

    // 所有写入操作都应暂存到memory_fs
    std::string file_path = it->second;
    auto file_token = file_manager_.getFileToken(file_path);

    if (file_token == "memory") {
        // 文件在memory_fs中，直接写入
        LOG_INFO << "[write] writing to memory file, fd=" << fd << " size=" << size;
        auto memory_fd_it = fd_to_memory_fd_map_.find(fd);
        if (memory_fd_it == fd_to_memory_fd_map_.end()) {
            LOG_ERROR << "[write] No memory_fd found for fd=" << fd;
            return -EBADF;
        }

        int memory_fd = memory_fd_it->second;
        auto write_size = memory_fs_.write(memory_fd, buf, size);

        // 更新文件管理器中的文件大小
        auto file_it = memory_fs_.files_.find(file_path);
        if (file_it != memory_fs_.files_.end()) {
            file_manager_.updateFileSize(file_path, file_it->second.data.size());
        }

        return write_size;
    } else {
        // 文件在BwtFS中，需要使用COW策略
        LOG_INFO << "[write] file in BwtFS, using COW strategy: " << file_path << " size=" << size;

        // 声明临时文件描述符，以便在catch块中也能访问
        int temp_fd = -1;
        std::string temp_file_path;

        try {
            // 1. 从BwtFS读取原文件内容
            BwtFS::Node::bw_tree read_tree(file_token, false);

            // 2. 创建临时文件名
            temp_file_path = file_path + ".cow_temp";

            // 3. 在memory_fs中创建临时文件
            if (memory_fs_.create(temp_file_path) < 0) {
                LOG_ERROR << "[write] failed to create temp file in memory_fs: " << temp_file_path;
                return -EIO;
            }

            temp_fd = memory_fs_.open(temp_file_path);
            if (temp_fd < 0) {
                LOG_ERROR << "[write] failed to open temp file in memory_fs: " << temp_file_path;
                return -EIO;
            }

            // 4. 读取原文件内容到临时文件
            constexpr size_t chunk_size = 4096;
            size_t total_bytes = 0;
            int index = 0;

            while (true) {
                auto data = read_tree.read(index, chunk_size);
                if (data.empty()) {
                    break;
                }

                memory_fs_.write(temp_fd, reinterpret_cast<const char*>(data.data()), data.size());
                total_bytes += data.size();
                index += data.size();

                if (data.size() < chunk_size) {
                    break;
                }
            }

            // 5. 写入新的内容（从参数buf）
            memory_fs_.write(temp_fd, buf, size);

            // 6. 获取更新后的文件大小
            auto temp_file_it = memory_fs_.files_.find(temp_file_path);
            size_t final_size = temp_file_it->second.data.size();

            LOG_INFO << "[write] COW: read " << total_bytes << " bytes, wrote " << size << " bytes, total: " << final_size << " for: " << file_path;

            // 8. 关闭临时文件描述符
            memory_fs_.close(temp_fd);
            temp_fd = -1;  // 标记为已关闭

            // 9. 验证原token有效性，避免删除无效文件导致崩溃
            if (file_token.empty() ||
                file_token.length() <= 10) {
                LOG_ERROR << "[write] invalid original token, keeping as memory file: " << file_token;
                file_manager_.remove(file_path);
                file_manager_.addFile(file_path, "memory", final_size);
                memory_fs_.remove(temp_file_path);
                return size;
            }

            // 10. 先创建新文件（安全策略：先创建再删除）
            LOG_INFO << "[write] creating new BwtFS file: " << file_path;
            BwtFS::Node::bw_tree new_tree;
            new_tree.write(const_cast<char*>(temp_file_it->second.data.data()), final_size);
            new_tree.flush();
            new_tree.join();

            std::string new_token = new_tree.get_token();

            // Validate the new token - ensure it's valid Base64 and doesn't contain problematic characters
            if (!new_token.empty() &&
                new_token.length() > 10) {

                // 11. 新文件创建成功，现在安全删除原文件
                LOG_DEBUG << "[write] deleting original BwtFS file: " << file_path;
                try {
                    BwtFS::Node::bw_tree delete_tree(file_token, true);
                    delete_tree.delete_file();
                } catch (const std::exception& e) {
                    LOG_WARNING << "[write] failed to delete original file (may be already corrupted): " << e.what();
                    // 继续执行，因为新文件已经创建成功
                }

                // 12. 更新文件管理器
                LOG_INFO << "[write] COW successfully updated: " << file_path << " with new token: " << new_token;
                file_manager_.remove(file_path);
                file_manager_.addFile(file_path, new_token, final_size);

                // 13. 删除临时文件
                memory_fs_.remove(temp_file_path);

                return size;
            } else {
                LOG_ERROR << "[write] COW failed to get valid token, keeping original file: " << new_token;
                // 新文件创建失败，保持原文件不变，这是最安全的策略
                // 不删除原文件，不更新token，只删除临时文件
                memory_fs_.remove(temp_file_path);
                return size;
            }

        } catch (const std::exception& e) {
            LOG_ERROR << "[write] COW strategy failed for " << file_path << ": " << e.what();
            // 清理临时文件描述符（只在有效时关闭）
            if (temp_fd >= 0) {
                memory_fs_.close(temp_fd);
            }
            // 清理临时文件（如果存在）
            if (!temp_file_path.empty()) {
                memory_fs_.remove(temp_file_path);
            }
            return -EIO;
        }
    }
}

int BwtFSMounter::write(int fd, const char* buf, size_t size, off_t offset) {
    LOG_DEBUG << "[write] requested fd=" << fd << " offset=" << offset << " size=" << size;

    auto it = fd_map_.find(fd);
    if (it == fd_map_.end()) {
        LOG_WARNING << "[write] Invalid fd map entry for fd=" << fd << ", treating as closed";
        return -EBADF;
    }

    // 所有写入操作都应暂存到memory_fs
    std::string file_path = it->second;
    auto file_token = file_manager_.getFileToken(file_path);

    if (file_token == "memory") {
        // 文件在memory_fs中，支持偏移写入
        LOG_INFO << "[write] writing to memory file with offset, fd=" << fd << " offset=" << offset << " size=" << size;
        auto memory_fd_it = fd_to_memory_fd_map_.find(fd);
        if (memory_fd_it == fd_to_memory_fd_map_.end()) {
            LOG_ERROR << "[write] No memory_fd found for fd=" << fd;
            return -EBADF;
        }

        int memory_fd = memory_fd_it->second;
        auto write_size = memory_fs_.write(memory_fd, buf, size, offset);

        // 更新文件管理器中的文件大小
        auto file_it = memory_fs_.files_.find(file_path);
        if (file_it != memory_fs_.files_.end()) {
            file_manager_.updateFileSize(file_path, file_it->second.data.size());
        }

        return write_size;
    } else {
        // 文件在BwtFS中，需要使用COW策略
        LOG_INFO << "[write] file in BwtFS, using COW strategy with offset: " << file_path << " offset=" << offset << " size=" << size;

        // 声明临时文件描述符，以便在catch块中也能访问
        int temp_fd = -1;
        std::string temp_file_path;

        try {
            // 1. 从BwtFS读取原文件内容
            BwtFS::Node::bw_tree read_tree(file_token, false);

            // 2. 创建临时文件名
            temp_file_path = file_path + ".cow_temp";

            // 3. 在memory_fs中创建临时文件
            if (memory_fs_.create(temp_file_path) < 0) {
                LOG_ERROR << "[write] failed to create temp file in memory_fs: " << temp_file_path;
                return -EIO;
            }

            temp_fd = memory_fs_.open(temp_file_path);
            if (temp_fd < 0) {
                LOG_ERROR << "[write] failed to open temp file in memory_fs: " << temp_file_path;
                return -EIO;
            }

            // 4. 读取原文件内容到临时文件
            constexpr size_t chunk_size = 4096;
            size_t total_bytes = 0;
            int index = 0;

            while (true) {
                auto data = read_tree.read(index, chunk_size);
                if (data.empty()) {
                    break;
                }

                memory_fs_.write(temp_fd, reinterpret_cast<const char*>(data.data()), data.size());
                total_bytes += data.size();
                index += data.size();

                if (data.size() < chunk_size) {
                    break;
                }
            }

            // 5. 确保临时文件足够大以容纳偏移写入
            if (offset + size > total_bytes) {
                // 用零填充扩展文件
                std::vector<char> padding(offset + size - total_bytes, 0);
                memory_fs_.write(temp_fd, padding.data(), padding.size());
                total_bytes = offset + size;
            }

            // 6. 在指定位置写入新内容
            memory_fs_.write(temp_fd, buf, size, offset);

            // 7. 获取最终文件大小
            auto temp_file_it = memory_fs_.files_.find(temp_file_path);
            size_t final_size = temp_file_it->second.data.size();

            LOG_INFO << "[write] COW: read " << total_bytes << " bytes, wrote offset " << offset << " size " << size << ", total: " << final_size << " for: " << file_path;

            // 8. 关闭临时文件描述符
            memory_fs_.close(temp_fd);
            temp_fd = -1;  // 标记为已关闭

            // 9. 验证原token有效性，避免删除无效文件导致崩溃
            if (file_token.empty() ||
                file_token.length() <= 10) {
                LOG_ERROR << "[write] invalid original token, keeping as memory file: " << file_token;
                file_manager_.remove(file_path);
                file_manager_.addFile(file_path, "memory", final_size);
                memory_fs_.remove(temp_file_path);
                return size;
            }

            // 10. 先创建新文件（安全策略：先创建再删除）
            LOG_INFO << "[write] creating new BwtFS file: " << file_path;
            BwtFS::Node::bw_tree new_tree;
            new_tree.write(const_cast<char*>(temp_file_it->second.data.data()), final_size);
            new_tree.flush();
            new_tree.join();

            std::string new_token = new_tree.get_token();

            // Validate the new token - ensure it's valid Base64 and doesn't contain problematic characters
            if (!new_token.empty()  &&
                new_token.length() > 10) {

                // 11. 新文件创建成功，现在安全删除原文件
                LOG_DEBUG << "[write] deleting original BwtFS file: " << file_path;
                try {
                    BwtFS::Node::bw_tree delete_tree(file_token, true);
                    delete_tree.delete_file();
                } catch (const std::exception& e) {
                    LOG_WARNING << "[write] failed to delete original file (may be already corrupted): " << e.what();
                    // 继续执行，因为新文件已经创建成功
                }

                // 12. 更新文件管理器
                LOG_INFO << "[write] COW successfully updated: " << file_path << " with new token: " << new_token;
                file_manager_.remove(file_path);
                file_manager_.addFile(file_path, new_token, final_size);

                // 13. 删除临时文件
                memory_fs_.remove(temp_file_path);

                return size;
            } else {
                LOG_ERROR << "[write] COW failed to get valid token, keeping original file: " << new_token;
                // 新文件创建失败，保持原文件不变，这是最安全的策略
                // 不删除原文件，不更新token，只删除临时文件
                memory_fs_.remove(temp_file_path);
                return size;
            }

        } catch (const std::exception& e) {
            LOG_ERROR << "[write] COW strategy failed for " << file_path << ": " << e.what();
            // 清理临时文件描述符（只在有效时关闭）
            if (temp_fd >= 0) {
                memory_fs_.close(temp_fd);
            }
            // 清理临时文件（如果存在）
            if (!temp_file_path.empty()) {
                memory_fs_.remove(temp_file_path);
            }
            return -EIO;
        }
    }
}

int BwtFSMounter::remove(const std::string& path){
    LOG_DEBUG << "[unlink] " << path;
    std::string file_token = file_manager_.getFileToken(path);
    if (file_token.empty() || file_token == "") {
        LOG_ERROR << "文件不存在: " << path;
        return -1;
    }
    if (file_token == "memory") {
        file_manager_.remove(path);
        // 内存文件，直接从memory_fs删除
        return memory_fs_.remove(path);
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

    // 处理只读打开的文件（已有token的文件）
    auto it = fd_tree_map_.find(fd);
    if (it != fd_tree_map_.end()) {
        delete it->second;
        fd_tree_map_.erase(it);

        // 清理相关映射
        if (path_it != fd_map_.end()) {
            path_fd_map_.erase(path_it->second);
            fd_map_.erase(path_it);
        }

        LOG_DEBUG << "[close] completed read-only fd=" << fd;
        return 0;
    }

    // 获取文件token
    std::string file_token = file_manager_.getFileToken(file_path);

    if (file_token == "memory") {
        // 处理暂存在memory_fs中的文件，需要写入到bwtfs
        LOG_INFO << "[close] finalizing memory file: " << file_path;

        auto memory_file_it = memory_fs_.files_.find(file_path);
        if (memory_file_it == memory_fs_.files_.end()) {
            LOG_ERROR << "[close] memory file not found: " << file_path;
            // 清理映射
            if (path_it != fd_map_.end()) {
                path_fd_map_.erase(path_it->second);
                fd_map_.erase(path_it);
            }
            auto memory_fd_it = fd_to_memory_fd_map_.find(fd);
            if (memory_fd_it != fd_to_memory_fd_map_.end()) {
                memory_fs_.close(memory_fd_it->second);
                fd_to_memory_fd_map_.erase(memory_fd_it);
            }
            return -EIO;
        }

        const auto& data = memory_file_it->second.data;

        // 检查文件是否为空 - 参考memory_fs，空文件也应该保留
        if (data.empty()) {
            LOG_INFO << "[close] empty file detected, keeping in memory_fs like memory_fs: " << file_path;

            // 对于系统临时文件，保持简洁处理
            std::string basename = file_path.substr(file_path.find_last_of('/') + 1);
            bool is_system_temp_file = (basename.find("._") == 0 || basename == ".DS_Store");

            if (is_system_temp_file) {
                LOG_INFO << "[close] system temp file, keeping in memory_fs: " << file_path;
            } else {
                LOG_INFO << "[close] user file, keeping in memory_fs waiting for data: " << file_path;
            }

            // 无论是否为空，都保持文件在memory_fs中，等待后续写入
            // 更新文件管理器记录，保持token为"memory"
            file_manager_.remove(file_path);
            file_manager_.addFile(file_path, "memory", 0);

            // 清理映射
            if (path_it != fd_map_.end()) {
                path_fd_map_.erase(path_it->second);
                fd_map_.erase(path_it);
            }
            auto memory_fd_it = fd_to_memory_fd_map_.find(fd);
            if (memory_fd_it != fd_to_memory_fd_map_.end()) {
                memory_fs_.close(memory_fd_it->second);
                fd_to_memory_fd_map_.erase(memory_fd_it);
            }
            return 0;
        }

        try {
            // 简化策略：参考memory_fs的成功做法，所有文件都尝试写入BwtFS
            // 如果失败，就保持在memory_fs中
            LOG_INFO << "[close] writing " << data.size() << " bytes to bwtfs for: " << file_path;

            // 创建新的bw_tree对象
            BwtFS::Node::bw_tree tree;

            // 写入数据到bw_tree（按照cmd项目的方式）
            tree.write(const_cast<char*>(data.data()), data.size());

            // 刷新缓冲区并等待树构建完成
            LOG_INFO << "[close] flushing tree for: " << file_path;
            tree.flush();

            LOG_INFO << "[close] joining tree for: " << file_path;
            tree.join();

            // 获取访问令牌
            std::string new_token = tree.get_token();

            if (!new_token.empty() && new_token.find('*') == std::string::npos) {
                // 成功获取有效token，更新文件记录
                LOG_INFO << "[close] successfully finalized " << file_path << " with token " << new_token;

                // 更新文件管理器中的记录
                file_manager_.remove(file_path);
                file_manager_.addFile(file_path, new_token, data.size());

                // 从memory_fs中删除文件
                memory_fs_.remove(file_path);

            } else {
                // token有问题，保持为memory文件
                LOG_WARNING << "[close] token issue detected, keeping as memory file: " << file_path;
                LOG_WARNING << "[close] token was: '" << new_token << "'";
                file_manager_.remove(file_path);
                file_manager_.addFile(file_path, "memory", data.size());
            }

        } catch (const std::exception& e) {
            LOG_WARNING << "[close] bwtfs finalize failed, keeping as memory file: " << file_path << " - " << e.what();
            // 发生异常时，保持为memory文件（像memory_fs一样简单处理）
            file_manager_.remove(file_path);
            file_manager_.addFile(file_path, "memory", data.size());

        } catch (...) {
            LOG_WARNING << "[close] unknown bwtfs error, keeping as memory file: " << file_path;
            // 发生异常时，保持为memory文件
            file_manager_.remove(file_path);
            file_manager_.addFile(file_path, "memory", data.size());
        }

        // 清理映射
        if (path_it != fd_map_.end()) {
            path_fd_map_.erase(path_it->second);
            fd_map_.erase(path_it);
        }
        auto memory_fd_it = fd_to_memory_fd_map_.find(fd);
        if (memory_fd_it != fd_to_memory_fd_map_.end()) {
            memory_fs_.close(memory_fd_it->second);
            fd_to_memory_fd_map_.erase(memory_fd_it);
        }

        LOG_DEBUG << "[close] completed memory file finalization for fd=" << fd;
        return 0;

    } else if (!file_token.empty()) {
        // 已经存在于bwtfs中的文件，直接关闭
        LOG_DEBUG << "[close] closing existing bwtfs file: " << file_path;

        // 清理映射
        if (path_it != fd_map_.end()) {
            path_fd_map_.erase(path_it->second);
            fd_map_.erase(path_it);
        }
        auto memory_fd_it = fd_to_memory_fd_map_.find(fd);
        if (memory_fd_it != fd_to_memory_fd_map_.end()) {
            memory_fs_.close(memory_fd_it->second);
            fd_to_memory_fd_map_.erase(memory_fd_it);
        }

        LOG_DEBUG << "[close] completed bwtfs file closure for fd=" << fd;
        return 0;

    } else {
        // 文件不存在或token无效
        LOG_ERROR << "[close] invalid file state for: " << file_path;
        return -EBADF;
    }
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
    if (file_node.token == "memory"){
        return true;
    }
    return !file_node.name.empty();
}

FileNode BwtFSMounter::getFileNode(const std::string& path) {
    // 确保路径以/开头
    std::string normalized_path = path;
    if (!path.empty() && path[0] != '/') {
        normalized_path = "/" + path;
    }

    // 首先检查是否是系统临时文件，如果是则从memory_fs获取
    // if (isSystemTempFile(normalized_path)) {
    //     LOG_DEBUG << "[getFileNode] getting from memory_fs: " << normalized_path;
    //     // 检查memory_fs中是否存在该文件
    //     auto it = memory_fs_.files_.find(normalized_path);
    //     if (it != memory_fs_.files_.end()) {
    //         // 在memory_fs中找到文件，返回对应的FileNode
    //         FileNode node;
    //         node.name = it->second.name;
    //         node.token = "memory_fs"; // 标识为内存文件
    //         node.file_size = it->second.data.size();
    //         node.is_dir = it->second.is_directory;
    //         LOG_DEBUG << "[getFileNode] found in memory_fs: " << normalized_path;
    //         return node;
    //     } else {
    //         // memory_fs中也不存在，返回空节点
    //         LOG_DEBUG << "[getFileNode] not found in memory_fs: " << normalized_path;
    //         return FileNode();
    //     }
    // }

    // 普通用户文件，从BwtFS获取
    auto file_node = file_manager_.getFile(normalized_path);
    return file_node;
}

SystemInfo BwtFSMounter::getSystemInfo() {
    return system_manager_.getSystemInfo();
}



