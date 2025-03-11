#include<iostream>
#include "file_system.hpp"

int main(void){
    BwtFS::FileSystem fs;
    fs.create_file("/tmp/test.txt");
    fs.delete_file("/tmp/test.txt");
    std::cout << "Hello, World!" << std::endl;
    return 0;
}