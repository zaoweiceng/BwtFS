#include "BwtFS.h"

using BwtFS::Util::Logger;

void init(){
    auto config = BwtFs::Config::getInstance();
    LOG_INFO << "BwtFS initialized.";
}