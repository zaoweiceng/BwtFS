#ifndef SYSTEM_FILE_H
#define SYSTEM_FILE_H
#include <string>
#include <vector>
#include <fstream>
#include "node/binary.h"
namespace BwtFS::System{
    class File {
        /*
        * 文件类
        * 用于物理文件的基本读写操作
        * @author: zaoweiceng
        * @data: 2025-03-30
        */
        public:
            File(const std::string& path);
            File(const File& other) = delete;
            File& operator=(const File& other) = delete;
            File(File&& other) = delete;
            File& operator=(File&& other) = delete;

            virtual BwtFS::Node::Binary read(unsigned long long index);
            virtual void write(unsigned long long index, const BwtFS::Node::Binary& data);
            
            virtual ~File() = default;

        private:
            std::fstream file;
    };
}
#endif