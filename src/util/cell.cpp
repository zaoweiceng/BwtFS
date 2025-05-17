#include "util/cell.h"
#include "util/random.h"
#include <cstddef>
#include "util/log.h"

BwtFS::Util::RCA::RCA(unsigned seed, BwtFS::Node::Binary& binary){
    this->seed = seed;
    this->binary = binary;
    this->rule = BwtFS::Util::RandNumbers<int>(binary.size(), seed, 0, 3);
}

void BwtFS::Util::RCA::forward(){
    for (size_t i = 0; i < this->binary.size(); i++){
        apply(this->binary[i], this->rule[i], true);
    }
}

void BwtFS::Util::RCA::backward(){
    for (size_t i = 0; i < this->binary.size(); i++){
        apply(this->binary[i], this->rule[i], false);
    }
}

void BwtFS::Util::RCA::XOR(std::byte& b){
    auto cb = std::to_integer<int>(b);
    b = std::byte{(unsigned char)((cb & 0b11110000) | ((cb & 0b00001111) ^ (0b11110000 & cb) >> 4))};
}

void BwtFS::Util::RCA::XOR_BACK(std::byte& b){
    auto cb = std::to_integer<int>(b);
    b = std::byte{(unsigned char)((cb & 0b11110000) | ((cb & 0b00001111) ^ (0b11110000 & cb) >> 4))};
}

inline int shift_left(int b) {
    return ((b >> 1) | ((b & 0b00000001) << 7));
}

inline int shift_right(int b) {
    return ((b << 1) | ((b & 0b10000000) >> 7));
}

void BwtFS::Util::RCA::SHIFT(std::byte& b){
    b = std::byte{(unsigned char)shift_left(std::to_integer<int>(b))};
}

void BwtFS::Util::RCA::SHIFT_BACK(std::byte& b){
    b = std::byte{(unsigned char)shift_right(std::to_integer<int>(b))};
}

inline int fd_swap(int b) {
    return ((b & 0b1100) | ((b & 0b0010) >> 1) | ((b & 0b0001) << 1));
}

void BwtFS::Util::RCA::FD(std::byte& b){
    auto cb = std::to_integer<int>(b) & 0xff;
    int first = (cb & 0b11110000) >> 4, second = cb & 0b00001111;
    if ((first & 0b1000) >> 3 != (first & 0b0100) >> 2) first = fd_swap(first);
    if ((second & 0b1000) >> 3 != (second & 0b0100) >> 2) second = fd_swap(second);
    b = std::byte{(unsigned char)((first << 4) | second)};
}

void BwtFS::Util::RCA::FD_BACK(std::byte& b){
    FD(b);
}

inline int td_nor(int b) {
    return ((b & 0b1100) | (~b & 0b0011));
}

void BwtFS::Util::RCA::TD(std::byte& b){
    auto cb = std::to_integer<int>(b) & 0xff;
    int first = (cb & 0b11110000) >> 4, second = cb & 0b00001111;
    if ((first & 0b1000) >> 3 != (first & 0b0100) >> 2) first = td_nor(first);
    if ((second & 0b1000) >> 3 != (second & 0b0100) >> 2) second = td_nor(second);
    b = std::byte{(unsigned char)((first << 4) | second)};
}

void BwtFS::Util::RCA::TD_BACK(std::byte& b){
    TD(b);
}

void BwtFS::Util::RCA::apply(std::byte& b, short operation, bool forward){
    switch (operation){
        case 0:
            if (forward) XOR(b);
            else XOR_BACK(b);
            break;
        case 1:
            if (forward) SHIFT(b);
            else SHIFT_BACK(b);
            break;
        case 2:
            if (forward) FD(b);
            else FD_BACK(b);
            break;
        case 3:
            if (forward) TD(b);
            else TD_BACK(b);
            break;
    }
}

void BwtFS::Util::RCA::setSeed(unsigned seed) { 
    this->seed = seed; 
    if (this->binary.size() == 0) {
        LOG_WARNING << "Binary is empty, cannot set seed";
        throw std::runtime_error("Binary is empty");
    }
    this->rule = BwtFS::Util::RandNumbers<int>(binary.size(), seed, 0, 3);
}

void BwtFS::Util::RCA::setBinary(BwtFS::Node::Binary& binary) { 
    this->binary = binary; 
}