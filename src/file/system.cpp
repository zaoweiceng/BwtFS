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
    auto config = BwtFS::Config::getInstance();
    return BwtFS::System::createBwtFS(config["system_file"]["path"], std::stoull(config["system_file"]["size"]), config["system_file"]["prefix"]);
}

bool BwtFS::System::createBwtFS(const std::string& path, size_t file_size){
    auto config = BwtFS::Config::getInstance();
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
    std::uniform_int_distribution<int> distribution_bitmap(1, (int)(0.4*block_count));
    std::uniform_int_distribution<int> distribution_bitmap_wear((int)(0.5*block_count), (int)(0.8*block_count));
    size_t bitmap = distribution_bitmap(generator);
    size_t bitmap_wear = distribution_bitmap_wear(generator);

    LOG_DEBUG << "Version: " << (int)version;
    LOG_DEBUG << "File size: " << file_size;
    LOG_DEBUG << "Block size: " << block_size;
    LOG_DEBUG << "Block count: " << block_count;
    LOG_DEBUG << "Create time: " << create_time;
    // LOG_DEBUG << "Modify time: " << modify_time;
    // LOG_DEBUG << "Bitmap size: " << bitmap_size;
    // LOG_DEBUG << "Bitmap start: " << bitmap;
    // LOG_DEBUG << "Bitmap wear start: " << bitmap_wear;

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
    // bitmap初始化
    BwtFS::System::Bitmap bitmap_obj(bitmap, bitmap_wear, bitmap_size, block_count, std::make_shared<BwtFS::System::File>(path_));
    bitmap_obj.init(block_count-1);
    LOG_DEBUG << "BwtFS system file initialized: " << path_;
    return true;
}

std::shared_ptr<BwtFS::System::FileSystem> BwtFS::System::openBwtFS(const std::string& path){
    auto path_ = std::filesystem::path(path).make_preferred().string();
    if (!std::filesystem::exists(path_)){
        LOG_ERROR << "File does not exist: " << path_;
        throw std::runtime_error(std::string("File does not exist: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    auto file = std::make_shared<BwtFS::System::File>(path_);
    unsigned block_count_ = (file->getFileSize() - sizeof(unsigned) - file->getPrefixSize()) / BwtFS::BLOCK_SIZE;
    auto auth_block = file->read(block_count_ - 1);
    auto modify_time = auth_block.read(0, sizeof(unsigned long long));
    auto hash_value = auth_block.read(sizeof(unsigned long long), sizeof(size_t));
    auto seed_of_cell = auth_block.read(sizeof(unsigned long long) + sizeof(size_t), sizeof(unsigned));
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
    if (reinterpret_cast<size_t&>(file_size[0]) == 0){
        LOG_ERROR << "File size is 0: " << path_;
        throw std::runtime_error(std::string("File size is 0: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    if (reinterpret_cast<unsigned&>(block_size[0]) == 0){
        LOG_ERROR << "Block size is 0: " << path_;
        throw std::runtime_error(std::string("Block size is 0: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    if (reinterpret_cast<unsigned&>(block_count[0]) == 0){
        LOG_ERROR << "Block count is 0: " << path_;
        throw std::runtime_error(std::string("Block count is 0: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    if (reinterpret_cast<unsigned long long&>(create_time[0]) == 0){
        LOG_ERROR << "Create time is 0: " << path_;
        throw std::runtime_error(std::string("Create time is 0: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    if (reinterpret_cast<size_t&>(bitmap[0]) == 0){
        LOG_ERROR << "Bitmap is 0: " << path_;
        throw std::runtime_error(std::string("Bitmap is 0: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    std::hash<std::string> hash_fn;
    size_t string_hash_value = hash_fn(system_info.to_hex_string());
    if (string_hash_value != reinterpret_cast<size_t&>(hash_value[0])){
        LOG_ERROR << "The system verification fails. The file system may be damaged or modified: " << path_;
        throw std::runtime_error(std::string("The system verification fails: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    LOG_INFO << "System verification passed: " << path_;
    LOG_INFO << "Last Modified: " << BwtFS::Util::timeToString(reinterpret_cast<unsigned long long&>(modify_time[0]));
    auto fs = BwtFS::System::FileSystem(file);
    fs.setHashValue(string_hash_value);
    fs.setSeedOfCell(reinterpret_cast<unsigned&>(seed_of_cell[0]));
    BwtFS::System::FS = std::make_shared<BwtFS::System::FileSystem>(file);
    return BwtFS::System::FS;
}

std::shared_ptr<BwtFS::System::FileSystem> BwtFS::System::getBwtFS(){
    if (BwtFS::System::FS == nullptr){
        LOG_ERROR << "File system is not initialized: " << __FILE__ << ":" << __LINE__;
        throw std::runtime_error(std::string("File system is not initialized: ") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    return BwtFS::System::FS;
}

BwtFS::System::FileSystem::FileSystem(std::shared_ptr<BwtFS::System::File> file){
    this->file = file;
    unsigned block_count_ = (file->getFileSize() - sizeof(unsigned) - file->getPrefixSize()) / BwtFS::BLOCK_SIZE;
    auto auth_block = file->read(block_count_ - 1);
    auto modify_time = auth_block.read(0, sizeof(unsigned long long));
    auto hash_value = auth_block.read(sizeof(unsigned long long), sizeof(size_t));
    auto seed_of_cell = auth_block.read(sizeof(unsigned long long) + sizeof(size_t), sizeof(unsigned));
    auto system_info = file->read(0);
    BwtFS::Util::RCA cell(reinterpret_cast<unsigned&>(seed_of_cell[0]), system_info);
    cell.backward();
    this->VERSION = reinterpret_cast<uint8_t&>(system_info.read(0, sizeof(uint8_t))[0]);
    this->FILE_SIZE = reinterpret_cast<size_t&>(system_info.read(sizeof(uint8_t), sizeof(size_t))[0]);
    this->BLOCK_SIZE = reinterpret_cast<unsigned&>(system_info.read(sizeof(uint8_t) + sizeof(size_t), sizeof(unsigned))[0]);
    this->BLOCK_COUNT = reinterpret_cast<unsigned&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned), sizeof(unsigned))[0]);
    this->CREATE_TIME = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2, sizeof(unsigned long long))[0]);
    this->BITMAP_START = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long), sizeof(size_t))[0]);
    this->BITMAP_WEAR_START = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long) * 2, sizeof(size_t))[0]);
    this->BITMAP_SIZE = reinterpret_cast<unsigned long long&>(system_info.read(sizeof(uint8_t) + sizeof(size_t) + sizeof(unsigned) * 2 + sizeof(unsigned long long) * 3, sizeof(size_t))[0]);
    this->is_open = true;
    this->MODIFY_TIME = reinterpret_cast<unsigned long long&>(modify_time[0]);
    this->bitmap = std::make_shared<BwtFS::System::Bitmap>(this->BITMAP_START, this->BITMAP_WEAR_START, this->BITMAP_SIZE, this->BLOCK_COUNT, file);
}

uint8_t BwtFS::System::FileSystem::getVersion() const{
    return this->VERSION;
}

size_t BwtFS::System::FileSystem::getFileSize() const{
    return this->FILE_SIZE;
}

size_t BwtFS::System::FileSystem::getBlockSize() const{
    return this->BLOCK_SIZE;
}

size_t BwtFS::System::FileSystem::getBlockCount() const{
    return this->BLOCK_COUNT;
}

unsigned long long BwtFS::System::FileSystem::getCreateTime() const{
    return this->CREATE_TIME;
}

unsigned long long BwtFS::System::FileSystem::getModifyTime() const{
    return this->MODIFY_TIME;
}

bool BwtFS::System::FileSystem::isOpen() const{
    return this->is_open;
}

size_t BwtFS::System::FileSystem::getBitmapSize() const{
    return this->BITMAP_SIZE;
}

bool BwtFS::System::FileSystem::check() const{
    unsigned block_count_ = (file->getFileSize() - sizeof(unsigned) - file->getPrefixSize()) / BwtFS::BLOCK_SIZE;
    auto auth_block = file->read(block_count_ - 1);
    auto modify_time = auth_block.read(0, sizeof(unsigned long long));
    auto hash_value = auth_block.read(sizeof(unsigned long long), sizeof(size_t));
    auto seed_of_cell = auth_block.read(sizeof(unsigned long long) + sizeof(size_t), sizeof(unsigned));
    auto system_info = file->read(0);
    BwtFS::Util::RCA cell(reinterpret_cast<unsigned&>(seed_of_cell[0]), system_info);
    cell.backward();
    std::hash<std::string> hash_fn;
    size_t string_hash_value = hash_fn(system_info.to_hex_string());
    if (string_hash_value != reinterpret_cast<size_t&>(hash_value[0])){
        return false;
    }
    return true;
}

size_t BwtFS::System::FileSystem::getFreeSize() const{
    return this->FILE_SIZE - this->bitmap->getSystemUsedSize();
}

size_t BwtFS::System::FileSystem::getFilesSize() const{
    return this->bitmap->getSystemUsedSize();
}

void BwtFS::System::FileSystem::updateModifyTime(){
    this->MODIFY_TIME = std::time(nullptr);
    BwtFS::Node::Binary binary(0);
    binary.append(sizeof(this->MODIFY_TIME), reinterpret_cast<std::byte*>(&this->MODIFY_TIME));
    binary.append(sizeof(this->STRING_HASH_VALUE), reinterpret_cast<std::byte*>(&STRING_HASH_VALUE));
    binary.append(sizeof(unsigned), reinterpret_cast<std::byte*>(&SEED_OF_CELL));
    this->file->write(this->BLOCK_COUNT - 1, binary);
}

void BwtFS::System::FileSystem::setHashValue(const size_t& hash_value){
    this->STRING_HASH_VALUE = hash_value;
}

void BwtFS::System::FileSystem::setSeedOfCell(unsigned seed_of_cell){
    this->SEED_OF_CELL = seed_of_cell;
}

BwtFS::Node::Binary BwtFS::System::FileSystem::read(const unsigned long long index){
    if (index > this->BLOCK_COUNT || index <= 0){
        LOG_ERROR <<  "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") 
        + __FILE__ + ":" + std::to_string(__LINE__));
    }
    std::shared_lock<std::shared_mutex> lock(rw_lock); // 共享锁，允许多个读取
    return this->file->read(index*BwtFS::BLOCK_SIZE);
}

void BwtFS::System::FileSystem::write(const unsigned long long index, const BwtFS::Node::Binary& data){
    if (index > this->BLOCK_COUNT || index < 0){
        LOG_ERROR <<  "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") 
        + __FILE__ + ":" + std::to_string(__LINE__));
    }
    if (data.size() < this->BLOCK_SIZE){
        LOG_WARNING << "Data size is less than block size: " 
        << data.size() << " < " << this->BLOCK_SIZE;
    }
    if (data.size() > this->BLOCK_SIZE){
        LOG_ERROR << "Data size is greater than block size: " 
        << data.size() << " > " << this->BLOCK_SIZE;
        throw std::out_of_range(std::string("Data size is greater than block size")
        + __FILE__ + ":" + std::to_string(__LINE__));
    }
    try{
        // 独占锁，只允许一个写入
        std::unique_lock<std::shared_mutex> lock(this->rw_lock);
        this->file->write(index*BwtFS::BLOCK_SIZE, data);
    }
    catch(const std::exception& e){
        LOG_ERROR << e.what();
        std::cerr << e.what() << '\n';
    }
}

