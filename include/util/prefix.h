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
        /*
        * 前缀类
        * 用于伪装文件的前缀
        * @author: zaoweiceng
        * @data: 2025-03-28
        */
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