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
    * +---------------+-------------------+-----------+
    * | prefix (可选) | data (数据)       | prefix大小|
    * +-------------------+-------------------+--------+
    * | * * * * * * * | ddddddddddddddd   | 0 0 0 8   |
    * +---------------+-------------------+-----------+
    *       prefix         data内容          prefix大小 
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
            virtual ~File() = default;
            // 读取数据
            // 传入块的位置，返回读取的数据
            virtual BwtFS::Node::Binary read(unsigned long long index);
            // 写入数据
            // 传入块的位置和数据，写入数据
            virtual void write(unsigned long long index, const BwtFS::Node::Binary& data);
            // 添加前缀
            // 传入前缀对象，添加前缀
            virtual void addPrefix(const std::string& prefix);
        private:
            // 文件对象
            std::shared_ptr<std::fstream> file;
            // 文件是否有前缀
            bool has_prefix;
            // 前缀对象
            std::shared_ptr<BwtFS::Util::Prefix> prefix;
            // prefix的大小
            size_t prefix_size;
    };
}
#endif