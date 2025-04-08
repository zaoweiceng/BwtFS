#ifndef SYSTEM_FILE_H
#define SYSTEM_FILE_H
#include <string>
#include <vector>
#include <fstream>
#include "node/binary.h"
#include "util/prefix.h"
namespace BwtFS::System{
    /*
    * 文件类
    * 用于物理文件的基本读写操作
    * @author: zaoweiceng
    * @data: 2025-03-30
    * 
    * 文件结构示例:
    * +---------------+--------------+-----------+
    * | prefix (可选) | data (数据)   | prefix大小|
    * +---------------+--------------+----------+
    * 
    * 
    */
    class File {
        public:
            // 构造函数
            // 传入文件路径，若文件不存在，则创建文件
            File(const std::string& path);
            File(const File& other) = delete;
            File& operator=(const File& other) = delete;
            File(File&& other) = delete;
            File& operator=(File&& other) = delete;
            ~File();
            // 读取数据
            // 传入块的位置，返回读取的数据
            BwtFS::Node::Binary read(unsigned long long index);
            // 读取数据
            // 传入块的位置和大小，返回读取的数据
            BwtFS::Node::Binary read(unsigned long long index, size_t size);
            // 写入数据
            // 传入块的位置和数据，写入数据
            void write(unsigned long long index, const BwtFS::Node::Binary& data);
            // 创建文件
            static unsigned createFile(const std::string& path, size_t size, std::string prefix = "");
            // 获取文件大小
            size_t getFileSize() const;
            // 获取文件prefix大小
            unsigned getPrefixSize() const;
            // 获取文件是否有前缀
            bool hasPrefix() const;
            // 关闭文件
            void close();

        private:
            // 文件对象
            std::shared_ptr<std::fstream> file;
            // 文件缓冲区对象
            std::filebuf* fb;
            // 文件是否有前缀
            bool has_prefix;
            // 前缀对象
            std::shared_ptr<BwtFS::Util::Prefix> prefix;
            // prefix的大小
            unsigned prefix_size;
            // 文件大小
            size_t file_size;
    };
}
#endif