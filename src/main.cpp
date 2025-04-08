#include "BwtFS.h"

int main(void){
    init();
    using BwtFS::Util::Logger;
    auto config = BwtFs::Config::getInstance();
    LOG_INFO <<  config["test"]["key"] ;

    BwtFS::System::File::createFile("O://test1.png", 256*BwtFS::MB, "O://pic.png");
    BwtFS::System::initBwtFS("O://test1.png");
    BwtFS::System::openBwtFS("O://test1.png");
    return 0;
}