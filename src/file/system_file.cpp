#include "file/system_file.h"
#include "util/ini_parser.h"
#include "util/log.h"
#include "config.h"
#include "util/prefix.h"
#include <filesystem>
#include <random>
#include <ctime>

using BwtFS::Util::Logger;
namespace fs = std::filesystem;

BwtFS::System::File::File(const std::string &path){
    auto path_ = fs::path(path).make_preferred().string();
    // 判断文件是否存在
    if (!fs::exists(path_)){
        auto config = BwtFs::Config::getInstance();
        LOG_INFO << "File does not exist: " << path_;
        LOG_INFO << "Creating file: " << path_;
        BwtFS::System::File::createFile(path_, std::stoull(config["system_file"]["size"]), config["system_file"]["prefix"]);
        LOG_INFO << "File created: " << path_;
    }
    LOG_DEBUG << "Opening file: " << path_;
    this->file = std::make_shared<std::fstream>();
    file->open(path_, std::ios::in | std::ios::out | std::ios::binary);
    if (!file->is_open()){
        LOG_ERROR << "Failed to open file: " << path_;
        throw std::runtime_error(std::string("Failed to open file: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    LOG_DEBUG << "File opened: " << path_;
    fb = file->rdbuf();
    if (fb == nullptr){
        LOG_ERROR << "Failed to open file: " << path_;
        throw std::runtime_error(std::string("Failed to open file: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    LOG_DEBUG << "File buffer opened: " << path_;
    this->file_size = fs::file_size(path_);
    LOG_DEBUG << "File size: " << this->file_size;
    fb->pubseekpos(this->file_size - sizeof(unsigned));
    fb->sgetn(reinterpret_cast<char*>(&this->prefix_size), sizeof(unsigned));
    LOG_DEBUG << "Prefix size: " << this->prefix_size;
    if (this->prefix_size == 0){
        this->has_prefix = false;
    }else{
        this->has_prefix = true;
    }
}

BwtFS::System::File::~File(){
    if (this->fb != nullptr){
        this->fb->close();
    }
    if (this->file != nullptr){
        this->file->close();
    }
}

unsigned BwtFS::System::File::createFile(const std::string& path, size_t size, std::string prefix){
    if (size < BwtFS::DefaultConfig::SYSTEM_FILE_MIN_SIZE){
        LOG_ERROR << "File size is too small: " << size << ". Minimum size is " << BwtFS::DefaultConfig::SYSTEM_FILE_MIN_SIZE;
        throw std::runtime_error(std::string("File size is too small: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    auto path_ = fs::path(path).make_preferred().string();
    auto parentPath = fs::path(path_).parent_path();
    if (!parentPath.empty()) {
        std::error_code ec;
        fs::create_directories(parentPath, ec);
        if (ec) {
            LOG_ERROR << "Failed to create config directories: " << ec.message();
            return 0;
        }
    }
    // 如果prefix不为空，则将path文件的后缀名改为prefix文件的后缀名
    if (prefix != ""){
        auto prefix_path = fs::path(prefix).make_preferred().string();
        auto path_suffix = fs::path(path_).extension();
        auto prefix_suffix = fs::path(prefix_path).extension();
        path_ = path_.substr(0, path_.size() - path_suffix.string().size()) + prefix_suffix.string();
        // if (path_suffix != prefix_suffix){
        //     fs::rename(path_, path_.substr(0, path_.size() - path_suffix.string().size()) + prefix_suffix.string());
        // }
    }
    if (fs::exists(path_)){
        LOG_ERROR << "File already exists: " << path_;
        throw std::runtime_error(std::string("File already exists: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    std::ofstream file(path_, std::ios::binary);
    if (!file.is_open()){
        LOG_ERROR << "Failed to create file: " << path_;
        return 0;
    }
    unsigned prefix_size = 0;
    if (prefix != ""){
        if (!fs::exists(prefix)){
            LOG_WARNING << "Prefix file is not exists: " << prefix;
            LOG_WARNING << "Do not use prefix file.";
            prefix = "";
        }else{
            auto p = BwtFS::Util::Prefix(prefix);
            prefix_size = p.size();
            if (file.is_open()){
                file.write(reinterpret_cast<const char*>(p.read(0, prefix_size).get()->data()), prefix_size);
            }else{
                LOG_ERROR << "Failed to open prefix file: " << prefix;
            }
        }
    }
    std::mt19937 rng(std::time(nullptr)); 
    std::uniform_int_distribution<std::uint8_t> dist(0, 255);
    const size_t bufferSize = 4096;
    std::vector<std::uint8_t> buffer(bufferSize);

    size_t remaining = size;
    while (remaining > 0) {
        size_t chunkSize = std::min(bufferSize, remaining);
        for (size_t i = 0; i < chunkSize; ++i) {
            buffer[i] = dist(rng);
        }
        file.write(reinterpret_cast<const char*>(buffer.data()), chunkSize);
        remaining -= chunkSize;
    }
    // 如果prefix不为空，则在文件末尾添加前缀大小
    if (prefix != ""){
        file.write(reinterpret_cast<const char*>(&prefix_size), sizeof(unsigned));
    }else{
        unsigned zero = 0;
        file.write(reinterpret_cast<const char*>(&zero), sizeof(unsigned));
    }
    file.close();
    LOG_INFO << "File created: " << path_;
    return prefix_size;
}

BwtFS::Node::Binary BwtFS::System::File::read(unsigned long long index_){
    auto index = index_ + this->prefix_size;
    if (index >= this->file_size){
        LOG_ERROR << "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    fb->pubseekpos(index);
    std::vector<std::byte> data(BwtFS::BLOCK_SIZE);
    fb->sgetn(reinterpret_cast<char*>(data.data()), BwtFS::BLOCK_SIZE);
    return BwtFS::Node::Binary(data);
}

BwtFS::Node::Binary BwtFS::System::File::read(unsigned long long index_, size_t size){
    auto index = index_ + this->prefix_size;
    if (index + size >= this->file_size){
        LOG_ERROR << "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    fb->pubseekpos(index);
    std::vector<std::byte> data(size*BwtFS::BLOCK_SIZE);
    fb->sgetn(reinterpret_cast<char*>(data.data()), size*BwtFS::BLOCK_SIZE);
    return BwtFS::Node::Binary(data);
}

void BwtFS::System::File::write(unsigned long long index_, const BwtFS::Node::Binary& data){
    auto index = index_ + this->prefix_size;
    if (index + data.size() >= this->file_size){
        LOG_ERROR << "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    fb->pubseekpos(index);
    fb->sputn(reinterpret_cast<const char*>(data.read().data()), data.size());
}

size_t BwtFS::System::File::getFileSize() const{
    return this->file_size;
}

unsigned BwtFS::System::File::getPrefixSize() const{
    return this->prefix_size;
}

bool BwtFS::System::File::hasPrefix() const{
    return this->has_prefix;
}

void BwtFS::System::File::close(){
    if (this->fb != nullptr){
        this->fb->close();
    }
    if (this->file != nullptr){
        this->file->close();
    }
}




