#include "gtest/gtest.h"
#include "file_system.hpp"
#include "util/random.h"
#include "BwtFS.h"

void test_bitmap_wear_balance() {
    init();
    using BwtFS::Util::Logger;
    auto config = BwtFs::Config::getInstance();
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
    auto config = BwtFs::Config::getInstance();
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
 
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    using BwtFS::Util::Logger;
    int res = RUN_ALL_TESTS();
    // test_bitmap_wear_balance(); // PASS
    // test_read_and_write();      // PASS
    return res;
}