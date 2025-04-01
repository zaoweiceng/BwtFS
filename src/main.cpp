#include "BwtFS.h"

int main(void){
    init();
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
    auto config = BwtFs::Config::getInstance();
    LOG_INFO <<  config["test"]["key"] ;
    
    // std::hash<std::string> hash_fn;
    // BwtFS::Node::Binary binary1("Hello World!AAABBBCCC1234567890", BwtFS::Node::StringType::ASCII);
    // std::size_t string_hash_value = hash_fn(binary1.to_hex_string());
    // LOG_INFO << "Hash value: " << string_hash_value;
    // BwtFS::Node::Binary binary2("Hello World!", BwtFS::Node::StringType::ASCII);
    // LOG_INFO << "Hash value: " << hash_fn(binary2.to_hex_string());

    return 0;
}