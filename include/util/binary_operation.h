#ifndef BINARY_OPERATION_HPP
#define BINARY_OPERATION_HPP
#include <vector>
#include <cstddef>
namespace BwtFS::Util::BinaryOperation{
    std::byte shift_left(std::byte b, std::size_t n);
    std::byte shift_right(std::byte b, std::size_t n);
};

#endif