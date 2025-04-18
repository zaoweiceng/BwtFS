#include "BwtFS.h"

using BwtFS::Util::Logger;

void init(){
    #ifdef _WIN32
    system("chcp 65001"); // 设置控制台编码为UTF-8
    #endif
    auto config = BwtFs::Config::getInstance();
    LOG_INFO << "BwtFS initialized.";
    LOG_CIALLO;
}