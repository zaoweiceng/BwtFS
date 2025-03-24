#include<iostream>
#include "file_system.hpp"
#include "util/binary_operation.hpp"
#include "node/binary.h"
#include <cstddef>
#include <string>

int main(void){
    std::string str = "00fff191", str1 = "00fff191";
    auto bin = BwtFS::Node::Binary(str, BwtFS::Node::StringType::ASCII);
    auto bin1 = BwtFS::Node::Binary(str1, BwtFS::Node::StringType::ASCII);
    auto bin2 = BwtFS::Node::Binary::contact({std::move(bin), std::move(bin1)});
    std::cout << bin2.to_base64_string() << std::endl;
    try{
        std::cout << bin1.to_base64_string() << std::endl;
    }catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    return 0;
}