#include "util/prefix.h"

BwtFS::Util::Prefix::Prefix(std::string src){
    this->src = src;
    this->fs.open(src, std::ios::in | std::ios::out | std::ios::binary);
    this->fb = this->fs.rdbuf();
}

BwtFS::Util::Prefix::~Prefix(){
    this->fs.close();
}

std::byte BwtFS::Util::Prefix::read(const size_t index){
    this->fb->pubseekpos(index);
    return static_cast<std::byte>(this->fb->sbumpc());
}

std::shared_ptr<std::vector<std::byte>> BwtFS::Util::Prefix::read(const size_t index, const size_t size){
    this->fb->pubseekpos(index);
    auto data = std::make_shared<std::vector<std::byte>>(size);
    this->fb->sgetn((char*)data->data(), size);
    return data;
}

size_t BwtFS::Util::Prefix::size(){
    return this->fb->pubseekoff(0, std::ios::end);
}