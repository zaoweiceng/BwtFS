#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include <cstddef>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "config.h"
#include "node/binary.h"
#include "util/prefix.h"
#include "file/bitmap.h"
#include "file/system_file.h"
namespace BwtFS::System{
    class FileSystem{
        /*
        * 文件系统类
        * 用于文件系统的读写操作
        * 维护文件系统的基本信息
        * @author: zaoweiceng
        * @data: 2025-03-30
        */
        friend BwtFS::System::Bitmap;
        public:
            FileSystem();
            FileSystem(const std::string& path);
            FileSystem(const FileSystem& other) = delete;
            FileSystem& operator=(const FileSystem& other) = delete;
            FileSystem(FileSystem&& other) = delete;
            FileSystem& operator=(FileSystem&& other) = delete;
            ~FileSystem() = default;

            // ------------ 文件系统操作 -------------
            // 创建文件系统
            const void NEW_FILESYSTEM(const std::string& path, size_t file_size);
            // 打开文件系统
            virtual void open();
            virtual void open(const std::string& path);
            // 关闭文件系统
            virtual void close();
            // 文件系统操作
            virtual void read(const unsigned long long index);
            virtual void write(const unsigned long long index, const BwtFS::Node::Binary& data);
            // 获取文件系统信息
            virtual void get_info();
            // 获取文件系统版本
            virtual uint8_t get_version() const;
            // 获取文件系统大小
            virtual size_t get_file_size() const;
            // 校验文件系统
            virtual bool check() const;

        private:
            uint8_t VERSION;
            size_t FILE_SIZE;
            size_t BLOCK_SIZE;
            size_t BLOCK_COUNT;
            unsigned long long CREATE_TIME;
            unsigned long long MODIFY_TIME;
            unsigned long long START_BLOCK;
            unsigned long long END_BLOCK;
            unsigned long long BITMAP_START;
            unsigned long long BITMAP_END;
            unsigned long long BITMAP_WEAR_START;
            unsigned long long BITMAP_WEAR_END;
            unsigned long long BITMAP_SIZE;
            size_t CHECHSUM;

            std::string path;
            std::string name;
            BwtFS::Node::Binary start_block;
            BwtFS::Node::Binary end_block;   
            
            BwtFS::System::File file;

            // 获取块大小
            virtual size_t get_block_size() const;
            // 获取块数量
            virtual size_t get_block_count() const;
            // 获取创建时间
            virtual unsigned long long get_create_time() const;
            // 获取修改时间
            virtual unsigned long long get_modify_time() const;
            // 获取起始块
            virtual unsigned long long get_start_block() const;
            // 获取结束块
            virtual unsigned long long get_end_block() const;
            // 获取位图起始块
            virtual unsigned long long get_bitmap_start() const;
            // 获取位图结束块
            virtual unsigned long long get_bitmap_end() const;
            // 获取位图大小
            virtual unsigned long long get_bitmap_size() const;
            // 获取位图磨损起始块
            virtual unsigned long long get_bitmap_wear_start() const;
            // 获取位图磨损结束块
            virtual unsigned long long get_bitmap_wear_end() const;
            // 获取位图磨损大小
            virtual unsigned long long get_bitmap_wear_size() const;
            // 获取校验和
            virtual size_t get_checksum() const;
    };

}
#endif