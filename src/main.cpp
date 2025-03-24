#include<iostream>
#include "file_system.hpp"
#include "util/binary_operation.h"
#include "node/binary.h"
#include "util/cell.h"
#include "util/random.h"
#include <cstddef>
#include <string>
void print(std::byte b){
    std::cout << "0b";
    for (int i = 7; i >= 0; --i) {
        std::cout << ((std::to_integer<int>(b) >> i) & 1);
    }
    std::cout << std::endl;
}

inline int td_nor(int b) {
    return ((b & 0b1100) | (~(b & 0b0011)));
}

int main(void){
    BwtFS::Node::Binary binary("Hello World!AAABBBCCC1234567890", BwtFS::Node::StringType::ASCII);
    std::cout << "Origin: " << binary.to_base64_string() << std::endl;
    BwtFS::Util::Cell cell(1024, binary);
    cell.forward();
    auto data = binary.to_base64_string();
    cell.backward();
    auto data1 = binary.to_base64_string();
    std::cout << "Backward: " << data1 << std::endl;
    std::cout << "Forward: " << data  << std::endl;
    return 0;
}