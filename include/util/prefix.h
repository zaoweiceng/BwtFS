#ifndef PREFIX_H
#define PREFIX_H
#include <string>
#include <fstream>
#include <iostream>
#include <cstddef>
#include <vector>
#include <memory>
namespace BwtFS::Util{
    /*
    * 前缀类
    * 用于伪装文件的前缀
    * @author: zaoweiceng
    * @data: 2025-03-28
    */
    class Prefix{
        public:
            // 构造函数
            // 传入伪装文件的路径
            // 传入伪装文件的路径，若文件不存在，则不使用伪装文件
            Prefix(std::string src);
            ~Prefix();
            // 读取数据
            // 传入索引，返回读取的数据
            std::byte read(const size_t index);
            // 读取数据
            // 传入索引和大小，返回读取的数据
            std::shared_ptr<std::vector<std::byte>> read(const size_t index, const size_t size);
            // 获取文件的大小
            size_t size();
        private:
            // 文件路径
            std::string src;
            // 文件对象
            std::filebuf* fb;
            // 文件对象
            std::fstream fs;
    };
};
#endif