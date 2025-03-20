#include<iostream>
#include "file_system.hpp"
#include "util/binary_operation.hpp"
#include <cstddef>

int main(void){
    std::byte d = std::byte{0b11};
    BwtFS::Util::BitwiseOperations<std::byte> b;
    std::cout << b.getBit(d, 0) << std::endl;
    b.setBit(d, 0);
    std::cout << b.getBit(d, 0) << std::endl;
    b.clearBit(d, 0);
    std::cout << b.getBit(d, 0) << std::endl;
    b.toggleBit(d, 0);
    std::cout << b.getBit(d, 0) << std::endl;
    return 0;
}