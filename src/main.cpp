#include "BwtFS.h"


int main(void){
    init();
    using BwtFS::Util::Logger;
    auto config = BwtFs::Config::getInstance();
    LOG_INFO <<  config["test"]["key"] ;

    try{
        // BwtFS::System::File::createFile("Z://test.bwt", 256*BwtFS::MB);
        // BwtFS::System::initBwtFS("Z://test.bwt");
        auto system = BwtFS::System::openBwtFS("Z://test.bwt");
        LOG_CIALLO;
        BwtFS::Node::bw_tree tree;
        namespace fs = std::filesystem;
        auto file = std::fstream();
        file.open("Z://crnn.pdf", std::ios::in | std::ios::out | std::ios::binary);
        char buffer[2048];
        while(true){
            file.read(buffer, sizeof(buffer));
            auto size = file.gcount();
            tree.write(buffer, size);
            if (size < sizeof(buffer)){
                break;
            }
        }
        tree.flush();
        file.close();
        // auto token = tree.generate_tree();
        tree.join();
        auto token = tree.get_token();
        LOG_INFO << "Token: " << token;



        LOG_INFO << system->getFreeSize() / BwtFS::MB << " MB";
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
    LOG_CIALLO;

    return 0;
}