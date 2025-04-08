#include "file/system.h"
#include "config.h"
#include "util/ini_parser.h"
#include "util/cell.h"
#include "util/log.h"
#include "util/date.h"
#include <cstdint>
#include <random>
#include <ctime>
#include <filesystem>

using BwtFS::Util::Logger;

bool BwtFS::System::createBwtFS(){
    auto config = BwtFs::Config::getInstance();
    return BwtFS::System::createBwtFS(config["system_file"]["path"], std::stoull(config["system_file"]["size"]), config["system_file"]["prefix"]);
}

bool BwtFS::System::createBwtFS(const std::string& path, size_t file_size){
    auto config = BwtFs::Config::getInstance();
    return BwtFS::System::createBwtFS(path, file_size, config["system_file"]["prefix"]) > 0;
}

bool BwtFS::System::createBwtFS(const std::string& path, size_t file_size, const std::string& prefix){
    try{
        BwtFS::System::File::createFile(path, file_size, prefix);
    }
    catch(const std::exception& e){
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;
}

bool BwtFS::System::initBwtFS(const std::string& path){
    auto path_ = std::filesystem::path(path).make_preferred().string();
    if (!std::filesystem::exists(path_)){
        LOG_ERROR << "File does not exist: " << path_;
        return false;
    }
    BwtFS::System::File file(path_);
    uint8_t version = BwtFS::VERSION;
    size_t file_size = file.getFileSize() - sizeof(unsigned) - file.getPrefixSize();
    unsigned block_size = BwtFS::BLOCK_SIZE;
    unsigned block_count = file_size / block_size;
    unsigned long long create_time = std::time(nullptr);
    unsigned long long modify_time = std::time(nullptr);
    size_t bitmap_size = block_count / 8 + 1;
    std::default_random_engine generator(create_time);
    std::uniform_int_distribution<int> distribution_bitmap((int)(0.2*block_count), (int)(0.5*block_count));
    std::uniform_int_distribution<int> distribution_bitmap_wear((int)(0.6*block_count), (int)(0.9*block_count));
    size_t bitmap = distribution_bitmap(generator);
    size_t bitmap_wear = distribution_bitmap_wear(generator);

    LOG_DEBUG << "Version: " << (int)version;
    LOG_DEBUG << "File size: " << file_size;
    LOG_DEBUG << "Block size: " << block_size;
    LOG_DEBUG << "Block count: " << block_count;
    LOG_DEBUG << "Create time: " << create_time;
    LOG_DEBUG << "Modify time: " << modify_time;
    LOG_DEBUG << "Bitmap size: " << bitmap_size;
    LOG_DEBUG << "Bitmap: " << bitmap;
    LOG_DEBUG << "Bitmap wear: " << bitmap_wear;

    BwtFS::Node::Binary binary(0);
    binary.append(sizeof(version), reinterpret_cast<std::byte*>(&version));
    binary.append(sizeof(file_size), reinterpret_cast<std::byte*>(&file_size));
    binary.append(sizeof(block_size), reinterpret_cast<std::byte*>(&block_size));
    binary.append(sizeof(block_count), reinterpret_cast<std::byte*>(&block_count));
    binary.append(sizeof(create_time), reinterpret_cast<std::byte*>(&create_time));
    binary.append(sizeof(bitmap), reinterpret_cast<std::byte*>(&bitmap));
    binary.append(sizeof(bitmap_wear), reinterpret_cast<std::byte*>(&bitmap_wear));
    binary.append(sizeof(bitmap_size), reinterpret_cast<std::byte*>(&bitmap_size));
    file.write(0, binary);
    auto data = file.read(0);
    std::hash<std::string> hash_fn;
    size_t string_hash_value = hash_fn(data.to_hex_string());
    LOG_DEBUG << "Hash value: " << string_hash_value;
    std::uniform_int_distribution<unsigned> random_distribution(0, std::numeric_limits<unsigned>::max());
    unsigned seed_of_cell = random_distribution(generator);
    BwtFS::Util::RCA cell(seed_of_cell, data);
    cell.forward();
    file.write(0, data);
    BwtFS::Node::Binary auth(0);
    auth.append(sizeof(modify_time), reinterpret_cast<std::byte*>(&modify_time));
    auth.append(sizeof(string_hash_value), reinterpret_cast<std::byte*>(&string_hash_value));
    auth.append(sizeof(unsigned), reinterpret_cast<std::byte*>(&seed_of_cell));
    file.write(block_count-1, auth);
    file.close();
    LOG_DEBUG << "BwtFS system file initialized: " << path_;
    return true;
}

BwtFS::System::FileSystem BwtFS::System::openBwtFS(const std::string& path){
    auto path_ = std::filesystem::path(path).make_preferred().string();
    if (!std::filesystem::exists(path_)){
        LOG_ERROR << "File does not exist: " << path_;
        throw std::runtime_error("File does not exist: " + path_);
    }
    auto file = std::make_shared<BwtFS::System::File>(path_);
    unsigned block_count_ = (file->getFileSize() - sizeof(unsigned) - file->getPrefixSize()) / BwtFS::BLOCK_SIZE;
    auto auth_block = file->read(block_count_ - 1);
    auto modify_time = auth_block.read(0, sizeof(unsigned long long));
    auto hash_value = auth_block.read(sizeof(unsigned long long), sizeof(size_t));
    auto seed_of_cell = auth_block.read(sizeof(unsigned long long) + sizeof(size_t), sizeof(unsigned));
    LOG_INFO << "Seed of cell: " << reinterpret_cast<unsigned&>(seed_of_cell[0]); // ----------------------------------
    auto system_info = file->read(0);
    BwtFS::Util::RCA cell(reinterpret_cast<unsigned&>(seed_of_cell[0]), system_info);
    cell.backward();
    auto version = system_info.read(0, sizeof(uint8_t));
    auto file_size = system_info.read(sizeof(uint8_t), sizeof(size_t));
    auto block_size = system_info.read(sizeof(uint8_t) + sizeof(size_t), sizeof(unsigned));
    auto block_count = system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned), sizeof(unsigned));
    auto create_time = system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2, sizeof(unsigned long long));
    auto bitmap = system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long), sizeof(size_t));
    auto bitmap_wear = system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long) * 2, sizeof(size_t));
    auto bitmap_size = system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long) * 3, sizeof(size_t));
    LOG_DEBUG << "BwtFS system file opened: " << path_;
    LOG_DEBUG << "Version: " << (int)reinterpret_cast<uint8_t&>(version[0]);
    LOG_DEBUG << "File size: " << reinterpret_cast<size_t&>(file_size[0]);
    LOG_DEBUG << "Block size: " << reinterpret_cast<unsigned&>(block_size[0]);
    LOG_DEBUG << "Block count: " << reinterpret_cast<unsigned&>(block_count[0]);
    LOG_DEBUG << "Create time: " << reinterpret_cast<unsigned long long&>(create_time[0]);
    LOG_DEBUG << "Bitmap: " << reinterpret_cast<size_t&>(bitmap[0]);
    LOG_DEBUG << "Bitmap wear: " << reinterpret_cast<size_t&>(bitmap_wear[0]);
    LOG_DEBUG << "Bitmap size: " << reinterpret_cast<size_t&>(bitmap_size[0]);
    if (reinterpret_cast<size_t&>(file_size[0]) == 0){
        LOG_ERROR << "File size is 0: " << path_;
        throw std::runtime_error("File size is 0: " + path_);
    }
    if (reinterpret_cast<unsigned&>(block_size[0]) == 0){
        LOG_ERROR << "Block size is 0: " << path_;
        throw std::runtime_error("Block size is 0: " + path_);
    }
    if (reinterpret_cast<unsigned&>(block_count[0]) == 0){
        LOG_ERROR << "Block count is 0: " << path_;
        throw std::runtime_error("Block count is 0: " + path_);
    }
    if (reinterpret_cast<unsigned long long&>(create_time[0]) == 0){
        LOG_ERROR << "Create time is 0: " << path_;
        throw std::runtime_error("Create time is 0: " + path_);
    }
    if (reinterpret_cast<size_t&>(bitmap[0]) == 0){
        LOG_ERROR << "Bitmap is 0: " << path_;
        throw std::runtime_error("Bitmap is 0: " + path_);
    }
    std::hash<std::string> hash_fn;
    size_t string_hash_value = hash_fn(system_info.to_hex_string());
    if (string_hash_value != reinterpret_cast<size_t&>(hash_value[0])){
        LOG_ERROR << "The system verification fails. The file system may be damaged or modified: " << path_;
        throw std::runtime_error("The system verification fails: " + path_);
    }
    LOG_INFO << "System verification passed: " << path_;
    LOG_INFO << "Last Modified: " << BwtFS::Util::timeToString(reinterpret_cast<unsigned long long&>(modify_time[0]));
    return BwtFS::System::FileSystem(file);
}

BwtFS::System::FileSystem::FileSystem(std::shared_ptr<BwtFS::System::File> file){
    this->file = file;
    auto system_info = file->read(0);
    this->VERSION = reinterpret_cast<uint8_t&>(system_info.read(0, sizeof(uint8_t))[0]);
    this->FILE_SIZE = reinterpret_cast<size_t&>(system_info.read(sizeof(uint8_t), sizeof(size_t))[0]);
    this->BLOCK_SIZE = reinterpret_cast<unsigned&>(system_info.read(sizeof(uint8_t) + sizeof(size_t), sizeof(unsigned))[0]);
    this->BLOCK_COUNT = reinterpret_cast<unsigned&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned), sizeof(unsigned))[0]);
    this->CREATE_TIME = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2, sizeof(unsigned long long))[0]);
    this->MODIFY_TIME = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long), sizeof(unsigned long long))[0]);
    this->BITMAP_START = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long) * 2, sizeof(unsigned long long))[0]);
    this->BITMAP_WEAR_START = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long) * 3, sizeof(unsigned long long))[0]);
    this->BITMAP_SIZE = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long) * 4, sizeof(unsigned long long))[0]);
    this->is_open = true;
}