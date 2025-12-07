#include "file/bitmap.h"
#include "config.h"
#include "util/log.h"
#include <algorithm>
#include <random>

using BwtFS::Util::Logger;

BwtFS::System::Bitmap::Bitmap(size_t bitmap_start, size_t bitmap_wear_start, size_t size, size_t bitmap_count, std::shared_ptr<BwtFS::System::File> file) {
    this->size = size;
    this->size_wear = size * 8;
    this->bitmap_start = bitmap_start;
    this->bitmap_wear_start = bitmap_wear_start;
    this->file = file;
    this->bitmap = file->read(bitmap_start*BwtFS::BLOCK_SIZE, this->size / BwtFS::BLOCK_SIZE + 1);
    this->bitmap_wear = file->read(bitmap_wear_start*BwtFS::BLOCK_SIZE, this->size_wear / BwtFS::BLOCK_SIZE + 1);
    this->bitmap_count = bitmap_count;
    this->init_bpm();
    // LOG_INFO << "Bitmap initialized." ;
}

void BwtFS::System::Bitmap::set(const size_t index) {
    // LOG_DEBUG << "index: " << index << " size: " << this->size;
    if (index >= this->size*8) {
        LOG_ERROR << "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    auto byte_index = index / 8;
    auto bit_index = index % 8;
    auto byte = (uint8_t)this->bitmap.get(byte_index);
    auto bit = (uint8_t)(byte | (1 << bit_index));
    this->bitmap.set(byte_index, std::byte(bit));
    this->save_bitmap();
    auto wear = (uint8_t)this->bitmap_wear.get(index);
    if (wear > 254) {
        LOG_WARNING << "Attempt to set a system block. This may cause system error.";
        return;
    }
    wear = wear + 1;
    if (wear > 250 && wear < 254) {
        this->wear_balance();
    }
    this->bitmap_wear.set(index, std::byte(wear));
    this->save_bitmap_wear();
}

void BwtFS::System::Bitmap::set_(const size_t index) {
    // LOG_DEBUG << "index: " << index << " size: " << this->size;
    if (index >= this->size*8) {
        LOG_ERROR << "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    auto byte_index = index / 8;
    auto bit_index = index % 8;
    auto byte = (uint8_t)this->bitmap.get(byte_index);
    auto bit = (uint8_t)(byte | (1 << bit_index));
    this->bitmap.set(byte_index, std::byte(bit));
    this->save_bitmap();
    auto wear = (uint8_t)this->bitmap_wear.get(index);
}

void BwtFS::System::Bitmap::clear(const size_t index) {
    // LOG_DEBUG << "index: " << index << " size: " << this->size;
    if (index >= this->size*8) {
        LOG_ERROR << "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    if ((uint8_t)this->bitmap_wear.get(index) > 254){
        LOG_WARNING << "Attempt to clear a system block. This may cause system error.";
        return;
    }
    auto byte_index = index / 8;
    auto bit_index = index % 8;
    auto byte = (uint8_t)this->bitmap.get(byte_index);
    auto bit = (uint8_t)(byte & ~(1 << bit_index));
    this->bitmap.set(byte_index, std::byte(bit));
    this->save_bitmap();
}

bool BwtFS::System::Bitmap::get(const size_t index) const {
    // LOG_DEBUG << "index: " << index << " size: " << this->size; 
    if (index >= this->size*8) {
        LOG_ERROR << "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    auto byte_index = index / 8;
    auto bit_index = index % 8;
    auto byte = (uint8_t)this->bitmap.get(byte_index);
    return (byte >> bit_index) & 1;
}

void BwtFS::System::Bitmap::wear_balance() {
    LOG_DEBUG << "Wear balance...";
    uint8_t min_wear = 255;
    for (size_t i = 0; i < this->bitmap_wear.size(); i++) {
        auto wear = (uint8_t)this->bitmap_wear.get(i);
        if (wear < min_wear) {
            min_wear = wear;
        }
    }
    if (min_wear == 0) {
        LOG_DEBUG << "Wear balance finished, no need to balance.";
        return;
    }
    for (size_t i = 0; i < this->bitmap_wear.size(); i++) {
        if ((uint8_t)this->bitmap_wear.get(i) < 254) {
            LOG_DEBUG << "Wear balance: " << i << " " << (int)this->bitmap_wear.get(i) << " -> " << (int)(min_wear-1);
            this->bitmap_wear.set(i, std::byte(min_wear-1));
        }
    }
}

void BwtFS::System::Bitmap::save_bitmap() {
    this->file->write(this->bitmap_start*BwtFS::BLOCK_SIZE, this->bitmap);
}

void BwtFS::System::Bitmap::save_bitmap_wear() {
    this->file->write(this->bitmap_wear_start*BwtFS::BLOCK_SIZE, this->bitmap_wear);
}

void BwtFS::System::Bitmap::save() {
    this->save_bitmap();
    this->save_bitmap_wear();
}

void BwtFS::System::Bitmap::init(unsigned last_index) {
    LOG_INFO << "Initializing bitmap...";
    for (size_t i = 0; i < this->size; i++) {
        this->bitmap.set(i, std::byte(0));
    }
    for (size_t i = 0; i < this->size_wear; i++) {
        this->bitmap_wear.set(i, std::byte(0));
    }
    for (size_t i = this->bitmap_start; i < this->bitmap_start + this->size / BwtFS::BLOCK_SIZE; i++) {
        this->bitmap_wear.set(i, std::byte(0));
        this->set_(i);
    }
    for (size_t i = this->bitmap_wear_start; i < this->bitmap_wear_start + this->size_wear / BwtFS::BLOCK_SIZE; i++) {
        this->bitmap_wear.set(i, std::byte(0));
        this->set_(i);
    }
    this->set_(0);
    this->set_(last_index);
    this->set_(last_index-1);
    this->bitmap_wear.set(0, std::byte(255));
    this->bitmap_wear.set(last_index, std::byte(255));
    this->bitmap_wear.set(last_index-1, std::byte(255));
    this->save();
}

uint8_t BwtFS::System::Bitmap::getWearBlock(const size_t index) const {
    if (index >= this->size*8) {
        LOG_ERROR << "Index out of range: " << index;
        throw std::out_of_range(std::string("Index out of range") + __FILE__ + ":" + std::to_string(__LINE__));
    }
    return (uint8_t)this->bitmap_wear.get(index);
}

size_t BwtFS::System::Bitmap::getSystemUsedSize() const {
    size_t used_size = 0, count = 0;
    for (size_t i = 0; i < this->bitmap.size(); i++) {
        auto byte = (uint8_t)this->bitmap.get(i);
        for (size_t j = 0; j < 8; j++) {
            if ((byte >> j) & 1) {
                used_size++;
            }
            count++;
            if (count >= this->bitmap_count) {
                return used_size*BwtFS::BLOCK_SIZE;
            }
        }
    }
    return used_size*BwtFS::BLOCK_SIZE;
}

void BwtFS::System::Bitmap::init_bpm(){
    // LOG_DEBUG << "Initializing bpm...";
    for (size_t i = 0; i < this->size-1; i++) {
        auto byte = (uint8_t)this->bitmap.get(i);
        for (size_t j = 0; j < 8; j++) {
            if ((byte >> j) & 1) {
                this->bpm.push_back({i*8+j, {true, (uint8_t)this->bitmap_wear.get(i*8+j)}});
            }else{
                this->bpm.push_back({i*8+j, {false, (uint8_t)this->bitmap_wear.get(i*8+j)}});
            }
        }
    }
    std::shuffle(this->bpm.begin(), this->bpm.end(), std::mt19937(std::random_device()()));
    std::sort(this->bpm.begin(), this->bpm.end(), [](const auto& a, const auto& b) {
        if (a.second.first == true && b.second.first == false) {
            return false;
        }else if (a.second.first == false && b.second.first == true) {
            return true;
        }else {
            return a.second.second < b.second.second;
        }
    });
    this->bpm_ptr = 0;
}

size_t BwtFS::System::Bitmap::getFreeBlock() {
    if (this->bpm_ptr >= this->bpm.size()) {
        this->init_bpm();
        if (this->bpm_ptr >= this->bpm.size()) {
            LOG_ERROR << "No free block available, system memory may used up or has some system error.";
            throw std::out_of_range(std::string("No free block available") + __FILE__ + ":" + std::to_string(__LINE__));
        }
    }
    auto block = this->bpm[this->bpm_ptr].first;
    this->bpm_ptr += 1;
    // for (size_t i = 0; i < this->bpm.size(); i++) {
    //     if(bpm[i].second.first == true){
    //         LOG_DEBUG << "bpm[" << i << "]: " << this->bpm[i].first << " " << (int)this->bpm[i].second.first << " " << (int)this->bpm[i].second.second;
    //     }
    // }
    if (this->get(block) == true){
        this->init_bpm();
        auto block = this->bpm[this->bpm_ptr].first;
        this->bpm_ptr += 1;
        if (this->get(block) == true){
            LOG_ERROR << "Block " << block << " is already used, system memory may used up.";
            return 0;
        }  
    }
    return block;
}

