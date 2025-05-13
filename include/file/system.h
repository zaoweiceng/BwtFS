#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <shared_mutex>
#include <thread>
#include "config.h"
#include "node/binary.h"
#include "util/prefix.h"
#include "file/bitmap.h"
#include "file/system_file.h"
namespace BwtFS::System{
    /*
    * 文件系统类
    * 用于文件系统的读写操作
    * 维护文件系统的基本信息
    * @author: zaoweiceng
    * @data: 2025-03-30
    * 
    * 
    * 
    * 
    * 系统节点存储内容结构表:
    *
    *   ±--------------±-----------------±--------------------±--------------+ 
    *   | 系统版本     | 系统大小        | 系统块大小          | 系统块数量     | 
    *   ±--------------±-----------------±--------------------±--------------+ 
    *   | 系统创建时间 | 系统位图起始位置 | 系统位图磨损起始位置| 系统位图大小     | 
    *   ±--------------±-----------------±--------------------±--------------+ 
    *   |                        系统数据部分                                 |
    *   ±--------------±-----------------±--------------------±--------------+ 
    *   | 系统修改时间 |    系统头校验    |    RCA_Seed         |(预留空间)    | 
    *   ±--------------±-----------------±--------------------±--------------+ 
    * 
    * 
    * 
    * 
    */
    class FileSystem{
        // 限制仅位图类能访问位图信息，其它类无权限访问和修改
        friend BwtFS::System::Bitmap;
        public:
            FileSystem() = delete;
            FileSystem(std::shared_ptr<BwtFS::System::File> file);
            FileSystem(const FileSystem& other) = delete;
            FileSystem& operator=(const FileSystem& other) = delete;
            FileSystem(FileSystem&& other) = delete;
            FileSystem& operator=(FileSystem&& other) = delete;
            ~FileSystem() = default;

            // // ------------ 文件系统操作 -------------
            // 文件系统操作
            virtual BwtFS::Node::Binary read(const unsigned long long index);
            virtual void write(const unsigned long long index, const BwtFS::Node::Binary& data);
            // 获取文件系统版本
            virtual uint8_t getVersion() const;
            // 获取文件系统大小
            virtual size_t getFileSize() const;
            // 校验文件系统
            virtual bool check() const;
            // 获取文件系统块大小
            virtual size_t getBlockSize() const;
            // 获取文件系统块数量
            virtual size_t getBlockCount() const;
            // 获取文件系统创建时间
            virtual unsigned long long getCreateTime() const;
            // 获取文件系统修改时间
            virtual unsigned long long getModifyTime() const;
            // 文件系统是否打开
            virtual bool isOpen() const;
            // 获取文件系统空闲空间
            virtual size_t getFreeSize() const;
            // 获取文件系统已用空间
            virtual size_t getFilesSize() const;
            
            size_t getBitmapSize() const;

            std::shared_ptr<BwtFS::System::Bitmap> bitmap;

            void setHashValue(const size_t& hash_value);

            void setSeedOfCell(unsigned seed_of_cell);

        private:
            // 文件系统版本
            uint8_t VERSION;
            // 文件系统大小
            size_t FILE_SIZE;
            // 文件系统块大小
            size_t BLOCK_SIZE;
            // 文件系统块数量
            size_t BLOCK_COUNT;
            // 文件系统创建时间
            unsigned long long CREATE_TIME;
            // 文件系统修改时间
            unsigned long long MODIFY_TIME;
            // 文件系统位图起始位置
            unsigned long long BITMAP_START;
            // 文件系统位图磨损起始位置
            unsigned long long BITMAP_WEAR_START;
            // 文件系统位图大小
            unsigned long long BITMAP_SIZE;
            // 文件系统字符串哈希值
            size_t STRING_HASH_VALUE; 
            // 文件系统随机数种子
            unsigned SEED_OF_CELL;

            // 文件系统是否打开
            bool is_open;
            // 文件系统对象
            std::shared_ptr<BwtFS::System::File> file;
            
            // 读写锁
            std::shared_mutex rw_lock;

            void updateModifyTime();
            
    };

    bool createBwtFS();
    bool createBwtFS(const std::string& path, size_t file_size);
    bool createBwtFS(const std::string& path, size_t file_size, const std::string& prefix);
    bool initBwtFS(const std::string& path);
    BwtFS::System::FileSystem openBwtFS(const std::string& path);

}
#endif