#include "util/cell.h"
#include "util/random.h"
#include "util/binary_operation.h"
#include <cstddef>

BwtFS::Util::Cell::Cell(unsigned seed, BwtFS::Node::Binary& binary){
    this->seed = seed;
    this->binary = binary;
    this->rule = BwtFS::Util::RandNumbers(binary.size(), seed, 0, 3);
}

void BwtFS::Util::Cell::forward(){
    for (size_t i = 0; i < this->binary.size(); i++){
        apply(this->binary[i], this->rule[i], true);
    }
}

void BwtFS::Util::Cell::backward(){
    for (size_t i = 0; i < this->binary.size(); i++){
        apply(this->binary[i], this->rule[i], false);
    }
}

void BwtFS::Util::Cell::XOR(std::byte& b){
    auto cb = std::to_integer<int>(b);
    b = std::byte{(unsigned char)((cb & 0b11110000) | ((cb & 0b00001111) ^ (0b11110000 & cb) >> 4))};
}

void BwtFS::Util::Cell::XOR_BACK(std::byte& b){
    auto cb = std::to_integer<int>(b);
    b = std::byte{(unsigned char)((cb & 0b11110000) | ((cb & 0b00001111) ^ (0b11110000 & cb) >> 4))};
}


void BwtFS::Util::Cell::SHIFT(std::byte& b){
    b = BwtFS::Util::BinaryOperation::shift_left(b, 1);
}

void BwtFS::Util::Cell::SHIFT_BACK(std::byte& b){
    b = BwtFS::Util::BinaryOperation::shift_right(b, 1);
}

inline int fd_swap(int b) {
    return ((b & 0b1100) | ((b & 0b0010) >> 1) | ((b & 0b0001) << 1));
}

void BwtFS::Util::Cell::FD(std::byte& b){
    auto cb = std::to_integer<int>(b) & 0xff;
    int first = (cb & 0b11110000) >> 4, second = cb & 0b00001111;
    if ((first & 0b1000) >> 3 != (first & 0b0100) >> 2) first = fd_swap(first);
    if ((second & 0b1000) >> 3 != (second & 0b0100) >> 2) second = fd_swap(second);
    b = std::byte{(unsigned char)((first << 4) | second)};
}

void BwtFS::Util::Cell::FD_BACK(std::byte& b){
    FD(b);
}

inline int td_nor(int b) {
    return ((b & 0b1100) | (~b & 0b0011));
}

void BwtFS::Util::Cell::TD(std::byte& b){
    auto cb = std::to_integer<int>(b) & 0xff;
    int first = (cb & 0b11110000) >> 4, second = cb & 0b00001111;
    if ((first & 0b1000) >> 3 != (first & 0b0100) >> 2) first = td_nor(first);
    if ((second & 0b1000) >> 3 != (second & 0b0100) >> 2) second = td_nor(second);
    b = std::byte{(unsigned char)((first << 4) | second)};
}

void BwtFS::Util::Cell::TD_BACK(std::byte& b){
    TD(b);
}

void BwtFS::Util::Cell::apply(std::byte& b, short operation, bool forward){
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