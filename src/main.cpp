#include<iostream>
#include "file_system.hpp"
#include "node/binary.h"
#include "util/cell.h"
#include "util/random.h"
#include <cstddef>
#include <string>
#include "util/prefix.h"
void print(std::byte b){
    std::cout << "0b";
    for (int i = 7; i >= 0; --i) {
        std::cout << ((std::to_integer<int>(b) >> i) & 1);
    }
    std::cout << std::endl;
}

int main(void){
    auto p = BwtFS::Util::Prefix("O://pic.png");
    size_t size = p.size();
    std::cout << size << std::endl;
    BwtFS::Node::Binary binary(p.read(0, size));
    BwtFS::Node::Binary binary1("hello world!", BwtFS::Node::StringType::ASCII);
    binary = binary + binary1;
    std::cout << binary.size() << std::endl;
    std::ofstream outFile("O://output.png", std::ios::binary | std::ios::out);
    if (outFile.is_open()) {
        outFile.write(reinterpret_cast<const char*>(binary.read().data()), binary.size());
        outFile.close();
        std::cout << "Binary data written to output.bin" << std::endl;
    } else {
        std::cerr << "Failed to open file for writing." << std::endl;
    }
    return 0;
}