#ifndef PREFIX_H
#define PREFIX_H
#include <string>
#include <fstream>
#include <iostream>
#include <cstddef>
#include <vector>
#include <memory>
namespace BwtFS::Util{

    class Prefix{
        public:
            Prefix(std::string src);
            ~Prefix();
            std::byte read(const size_t index);
            std::shared_ptr<std::vector<std::byte>> read(const size_t index, const size_t size);
            size_t size();
        private:
            std::string src;
            std::filebuf* fb;
            std::fstream fs;
    };
};
#endif