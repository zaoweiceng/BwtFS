#include "gtest/gtest.h"
#include "util/random.h"
#include "BwtFS.h"
#include <filesystem>
#include <chrono>

void test_bitmap_wear_balance() {
    init();
    using BwtFS::Util::Logger;
    auto config = BwtFS::Config::getInstance();
    try{
        // BwtFS::System::File::createFile("O://test2.png", 256*BwtFS::MB, "O://pic.png");
        // BwtFS::System::initBwtFS("O://test2.png");
        auto system = BwtFS::System::openBwtFS("O://test2.png");
        LOG_INFO << system->getFreeSize() / BwtFS::MB << " MB";
        bool flag = false;
        int count = 65535*3;
        while(count--){
            if (!flag){
                auto t = system->bitmap->getFreeBlock();
                if (t == 0){
                    LOG_ERROR << "No free block available, system memory may used up.";
                    flag = true;
                    continue;
                }
                system->bitmap->set(t);
            }else{
                LOG_INFO << "Clear bitmap wear balance...";
                system->getBitmapSize();
                auto rv = BwtFS::Util::RandNumbers(20000, count, 0, system->getBlockCount()-1);
                for (auto i : rv){
                    if (system->bitmap->getWearBlock(i) > 254){
                        LOG_WARNING << "Attempt to clear a system block. Index: " << i << " Wear: " << (int)system->bitmap->getWearBlock(i);
                        continue;
                    }
                    system->bitmap->clear(i);
                }
                flag = false;
            }
        }
    }catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void test_read_and_write() {
    init();
    using BwtFS::Util::Logger;
    auto config = BwtFS::Config::getInstance();
    try{
        // BwtFS::System::File::createFile("O://test.png", 256*BwtFS::MB, "O://pic.png");
        // BwtFS::System::initBwtFS("O://test.png");
        auto system = BwtFS::System::openBwtFS("O://test.png");
        LOG_INFO << system->getFreeSize() / BwtFS::MB << " MB";
        auto data = BwtFS::Node::Binary("Hello World!", BwtFS::Node::StringType::ASCII);
        auto bitmap = system->bitmap->getFreeBlock();
        LOG_INFO << "Free block: " << bitmap;
        system->bitmap->set(bitmap);
        system->write(bitmap, data);
        auto read_data = system->read(bitmap);
        LOG_INFO << "Read data: " << read_data.to_ascll_string();
    }catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}
 

void write_test_file(const std::string& file_path) {
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + file_path);
    }
    const char* data = "Hello World!";
    int size = 1024*1024*64; 
    int count = size / 12; // "Hello World!" is 12 bytes
    for (int i = 0; i < count; ++i) {
        file.write(data, 12);
    }
    file.close();
    LOG_INFO << "Test file written at: " << file_path;
}

std::string write_test_file_in_bwt(){
    auto system = BwtFS::System::getBwtFS();
    BwtFS::Node::bw_tree tree;
    const char* data = "Hello World!";
    int size = 1024*1024*64;
    int count = size / 12; // "Hello World!" is 12 bytes
    for (int i = 0; i < count; ++i) {
        tree.write(const_cast<char *>(data), 12);
    }
    tree.flush();
    tree.join();
    LOG_INFO << "Test file written in BwtFS.";
    return tree.get_token();
}


void test_read_speed(std::fstream& file){
    int file_size = file.tellg();
    // 读取文件随机位置4KB大小的空间
    const int read_size = 100; // 4KB
    char* buffer = new char[read_size];
    int read_count = 10000;
    for (int i = 0; i < read_count; ++i) {
        BwtFS::Node::Binary bin;
        int offset = rand() % (file_size - read_size);
        file.seekg(offset);
        file.read(buffer, read_size);
        bin.append(read_size, reinterpret_cast<std::byte*>(buffer));
    }
}

// void test_read_speed(std::fstream& file){
//     int file_size = file.tellg();
//     const int read_size = 100; // 4KB
//     char buffer[4096];
//     Binary bin;
//     while(true){
//         file.read(buffer, sizeof(buffer));
//         auto size = file.gcount();
//         bin.append(size, reinterpret_cast<std::byte*>(buffer));
//         if (size < sizeof(buffer)){
//             break;
//         }
//     }  
// }

void test_read_speed_in_bwt(BwtFS::Node::bw_tree& tree) {
    int read_count = 10000; 
    for (int i = 0; i < read_count; ++i) {
        int offset = rand() % (1024*1024*64 - 100);
        offset = (offset / 4095) * 4095; // 确保偏移量是4KB的倍数
        auto data = tree.read(offset, 100); // 读取4KB数据
    }
}

// void test_read_speed_in_bwt(BwtFS::Node::bw_tree& tree) {
//     Binary bin;
//     size_t offset = 0;
//     while(true){
//         auto data = tree.read(offset, 4095); // 读取4KB数据
//         offset += data.size();
//         bin.append(data.size(), reinterpret_cast<std::byte*>(data.data()));
//         if (data.size() < 4095){
//             break;
//         }
//     }
// }

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int res = RUN_ALL_TESTS();
    return res;
    using BwtFS::Util::Logger;
    
    // init();
    // std::string file_path = "Z://test.bwt";
    // if(!std::filesystem::exists(file_path)) {
    //     try {
    //         BwtFS::System::File::createFile(file_path, 1*BwtFS::GB);
    //         BwtFS::System::initBwtFS(file_path);
    //         LOG_INFO << "Test file created at: " << file_path;
    //     } catch (const std::exception& e) {
    //         LOG_ERROR << "Failed to create test file: " << e.what();
    //         return 1;
    //     }
    // } else {
    //     LOG_INFO << "Test file already exists at: " << file_path;
    // }
    // auto system = BwtFS::System::openBwtFS(file_path);
    // if (system == nullptr) {
    //     LOG_ERROR << "Failed to open BwtFS system.";
    //     return 1;
    // }
    // LOG_INFO << "BwtFS system opened successfully.";
    // // write_test_file("Z://test_file.txt");
    // std::string token = "G2g0aAAAAAA=NRyaaaaaaadlbrahvZud";
    // try{
    //     token = write_test_file_in_bwt();
    //     LOG_INFO << "Test file written in BwtFS with token: " << token;
    // }catch (const std::exception& e) {
    //     LOG_ERROR << "Failed to write test file in BwtFS: " << e.what();
    //     return 1;
    // }
    // 测试读取速度
    // std::fstream file("Z://test_file.txt", std::ios::in | std::ios::binary);
    // if (!file.is_open()) {
    //     LOG_ERROR << "Failed to open test file for reading.";
    //     return 1;
    // }
    // LOG_INFO << "Testing read speed from file...";
    // auto start = std::chrono::high_resolution_clock::now();
    // test_read_speed(file);
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // LOG_INFO << "Read speed test completed in " << duration.count() << " ms.";
    // file.close();
    // LOG_INFO << "Testing read speed from BwtFS...";
    // BwtFS::Node::bw_tree tree(token);
    // LOG_INFO << "File opened in BwtFS with token: " << token;
    // start = std::chrono::high_resolution_clock::now();
    // test_read_speed_in_bwt(tree);
    // end = std::chrono::high_resolution_clock::now();
    // duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // LOG_INFO << "Read speed from BwtFS completed in " << duration.count() << " ms.";
    // LOG_INFO << "Read speed tests completed.";
    // test_bitmap_wear_balance(); // PASS
    // test_read_and_write();      // PASS
    return 0;
}