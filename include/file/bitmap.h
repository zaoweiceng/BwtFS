#ifndef BITMAP_H
#define BITMAP_H
#include <cstddef>
#include <string>
#include <vector>
#include "node/binary.h"
#include "file/system_file.h"
namespace BwtFS::System{
    /*
    * 位图类
    * 用于位图的读写操作
    * @author: zaoweiceng
    * @data: 2025-03-30
    */
    class Bitmap{
        public:
            // 构造函数
            // 传入文件对象，根据文件对象自动解析Bitmap
            Bitmap(BwtFS::System::File& file);
            Bitmap(const Bitmap& other) = delete;
            Bitmap& operator=(const Bitmap& other) = delete;
            Bitmap(Bitmap&& other) = delete;
            ~Bitmap() = default;
            // 设置指定位置位图的值
            // 传入索引和值，设置位图的值
            void set(const size_t index, const bool value);
            // 获取指定位置位图的值
            // 传入索引，返回位图的值
            bool get(const size_t index) const;
            


        private:
            // 初始化位图
            void init(const size_t size);
            // 位图
            BwtFS::Node::Binary bitmap;
            // 磨损位图
            BwtFS::Node::Binary bitmap_wear;
            // 位图的大小
            size_t size;
            // 磨损位图的大小
            size_t size_wear;
            // 文件对象
            BwtFS::System::File* file;
    };
}

#endif