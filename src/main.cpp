#include<iostream>
#include "file_system.hpp"
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

int main(void){
    BwtFS::Node::Binary binary("Hello World!", BwtFS::Node::StringType::ASCII);
    std::cout << "Origin: " << binary.to_base64_string() << std::endl;
    BwtFS::Util::RCA cell(4096, binary);
    cell.forward();
    auto data = binary.to_base64_string();
    cell.backward();
    auto data1 = binary.to_base64_string();
    std::cout << "Backward: " << data1 << std::endl;
    std::cout << "Forward: " << data  << std::endl;
    // auto b = std::byte{0b11010001};
    return 0;
}