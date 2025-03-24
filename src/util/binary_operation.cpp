#include "util/binary_operation.h"
#include <cstddef>
#include <climits>

std::byte BwtFS::Util::BinaryOperation::shift_left(std::byte b, std::size_t n) {
    static const std::size_t num_bits = CHAR_BIT; 
    n %= num_bits; 
    unsigned char uc = static_cast<unsigned char>(b);
    return static_cast<std::byte>((uc << n) | (uc >> (num_bits - n)));
}

std::byte BwtFS::Util::BinaryOperation::shift_right(std::byte b, std::size_t n) {
    static const std::size_t num_bits = CHAR_BIT; 
    n %= num_bits; 
    unsigned char uc = static_cast<unsigned char>(b);
    return static_cast<std::byte>((uc >> n) | (uc << (num_bits - n)));
}