#include "BwtFS.h"

int main(void){
    init();
    using BwtFS::Util::Logger;
    auto config = BwtFs::Config::getInstance();
    LOG_INFO <<  config["test"]["key"] ;

    try{
        // BwtFS::System::File::createFile("O://test2.png", 256*BwtFS::MB, "O://pic.png");
        // BwtFS::System::initBwtFS("O://test2.png");
        auto system = BwtFS::System::openBwtFS("O://test2.png");
        LOG_INFO << "Version: " << (int)system.getVersion();
        LOG_INFO << "File size: " << system.getFileSize();
        LOG_INFO << "Block size: " << system.getBlockSize();
        LOG_INFO << "Block count: " << system.getBlockCount();
        LOG_INFO << "Create time: " << system.getCreateTime();
        LOG_INFO << "Modify time: " << system.getModifyTime();


        LOG_INFO << system.getFreeSize() / BwtFS::MB << " MB";
        // for (int i = 0; i < 65535; i++){
        //     auto t = system.bitmap->getFreeBlock();
        //     if (t == 0){
        //         LOG_ERROR << "No free block available, system memory may used up.";
        //     }
        //     system.bitmap->set(t);
        // }
        

        
    }catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
    return 0;
}