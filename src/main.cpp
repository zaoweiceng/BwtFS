#include<iostream>
#include "file_system.hpp"
#include "node/binary.h"
#include "util/cell.h"
#include "util/random.h"
#include <cstddef>
#include <string>
#include "util/prefix.h"
#include "util/log.h"
#include <functional>

void print(std::byte b){
    std::cout << "0b";
    for (int i = 7; i >= 0; --i) {
        std::cout << ((std::to_integer<int>(b) >> i) & 1);
    }
    std::cout << std::endl;
}

int main(void){
    // auto p = BwtFS::Util::Prefix("O://pic.png");
    // size_t size = p.size();
    // std::cout << size << std::endl;
    // BwtFS::Node::Binary binary(p.read(0, size));
    // BwtFS::Node::Binary binary1("hello world!", BwtFS::Node::StringType::ASCII);
    // binary = binary + binary1;
    // std::cout << binary.size() << std::endl;
    // std::ofstream outFile("O://output.png", std::ios::binary | std::ios::out);
    // if (outFile.is_open()) {
    //     outFile.write(reinterpret_cast<const char*>(binary.read().data()), binary.size());
    //     outFile.close();
    //     std::cout << "Binary data written to output.bin" << std::endl;
    // } else {
    //     std::cerr << "Failed to open file for writing." << std::endl;
    // }
    using BwtFS::Util::Logger;
    using BwtFS::Util::LogLevel;
    Logger::getInstance().setLevel(LogLevel::DEBUG);
    Logger::getInstance().setConsole(true);
    Logger::getInstance().setFile(true, "app.log");
    
    LOG(LogLevel::DEBUG) << "This is a debug message.";
    LOG(LogLevel::INFO) << "User logged in. ID: " << 12345;
    LOG(LogLevel::WARNING) << "User tried to access a protected resource.";
    LOG(LogLevel::ERROR) << "Failed to open file.";
    LOG_INFO << "This is an info message.";
    LOG_WARNING << "This is a warning message.";
    LOG_ERROR << "This is an error message.";
    LOG_DEBUG << "This is a debug message.";
    
    std::hash<std::string> hash_fn;
    BwtFS::Node::Binary binary1("Hello World!AAABBBCCC1234567890", BwtFS::Node::StringType::ASCII);
    std::size_t string_hash_value = hash_fn(binary1.to_hex_string());
    LOG_INFO << "Hash value: " << string_hash_value;
    BwtFS::Node::Binary binary2("Hello World!", BwtFS::Node::StringType::ASCII);
    LOG_INFO << "Hash value: " << hash_fn(binary2.to_hex_string());

    return 0;
}