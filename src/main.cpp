#include "BwtFS.h"


int main(void){
    init();
    using BwtFS::Util::Logger;
    auto config = BwtFS::Config::getInstance();

    try{
        // BwtFS::System::File::createFile("Z://test.bwt", 256*BwtFS::MB);
        // BwtFS::System::initBwtFS("Z://test.bwt");
        auto system = BwtFS::System::openBwtFS("Z://test.bwt");
        LOG_CIALLO;
        BwtFS::Node::bw_tree tree;
        namespace fs = std::filesystem;
        auto file = std::fstream();
        file.open("Z://crnn.pdf", std::ios::in | std::ios::out | std::ios::binary);
        char buffer[4096];
        while(true){
            file.read(buffer, sizeof(buffer));
            // LOG_INFO << std::string(buffer, file.gcount());
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
        
        LOG_INFO << "Start reading data";
        BwtFS::Node::bw_tree tree2(token);
        auto file2 = std::fstream();
        file2.open("Z://test.txt", std::ios::out | std::ios::binary);
        int index = 0;
        // LOG_INFO << "Initial index: " << index;
        while(true){
            auto data = tree2.read(index, 4096);
            // LOG_INFO << "Data size: " << data.size();
            file2.write(reinterpret_cast<const char*>(data.data()), data.size());
            // LOG_INFO << reinterpret_cast<const char*>(data.data());
            index += data.size();
            if (data.size() < 2048){
                break;
            }
        }

         
    }catch(const std::exception& e){
        std::cerr << e.what() << '\n';
    }
    LOG_CIALLO;

    return 0;
}