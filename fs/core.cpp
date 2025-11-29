#include "core.h"
#include <cstring>
#include <iostream>

int MyFS::open(const std::string& path){
    if (files_.find(path) == files_.end()) {
        // 如果文件不存在，则创建
        files_[path] = File{path, {}};
    }
    int fd = next_fd_++;
    fd_map_[fd] = path;
    std::cout << "[open] " << path << " -> fd=" << fd << std::endl;
    return fd;
}

int MyFS::create(const std::string& path) {
    // 如果文件不存在，则创建一个空文件
    if (files_.find(path) == files_.end()) {
        files_[path] = File{path, {}};
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

    if (offset >= f.data.size()) {
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

std::vector<std::string> MyFS::list_files(){
    std::vector<std::string> res;
    for (auto& [name, _] : files_)
        res.push_back(name.substr(1)); // 去掉 '/'
    return res;
}
