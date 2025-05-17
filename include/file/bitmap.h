#ifndef BITMAP_H
#define BITMAP_H
#include <cstddef>
#include <string>
#include <vector>
#include <utility>
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
            Bitmap(size_t bitmap_start, size_t bitmap_wear_start, size_t size, size_t bitmap_count, std::shared_ptr<BwtFS::System::File> file);
            Bitmap(const Bitmap& other) = delete;
            Bitmap& operator=(const Bitmap& other) = delete;
            Bitmap(Bitmap&& other) = delete;
            ~Bitmap() = default;
            // 设置指定位置位图的值
            // 传入索引和值(第index个块)，设置位图的值
            void set(const size_t index);
            // 清空指定位置位图的值
            void clear(const size_t index);
            // 获取指定位置位图的值
            // 传入索引(第index个块)，返回位图的值
            bool get(const size_t index) const;
            // 获取一个空闲块
            // 返回空闲块的索引
            // 0表示没有空闲块
            // 大于0表示有空闲块，其值为空闲块的索引
            size_t getFreeBlock();
            // 获取指定块的磨损值
            // 传入索引(第index个块)，返回磨损值
            uint8_t getWearBlock(const size_t index) const;
            // 初始化位图
            void init(unsigned last_index);
            // 获取系统大小
            size_t getSystemUsedSize() const;


        private:
            // 位图起始位置
            unsigned long long bitmap_start;
            // 磨损位图起始位置
            unsigned long long bitmap_wear_start;
            // 位图
            BwtFS::Node::Binary bitmap;
            // 磨损位图
            BwtFS::Node::Binary bitmap_wear;
            // 位图的大小 Byte
            size_t size;
            // 磨损位图的大小 Byte
            size_t size_wear;
            // 位图个数
            size_t bitmap_count;
            // 文件对象
            std::shared_ptr<BwtFS::System::File> file;
            // BPM
            std::vector<std::pair<size_t, std::pair<bool, uint8_t>>> bpm;

            void set_(const size_t index);

            // 初始化BPM
            void init_bpm();
            // bpm指针
            size_t bpm_ptr;

            void save();
            // 保存位图
            void save_bitmap();
            // 保存磨损位图
            void save_bitmap_wear();
            // 均衡磨损
            void wear_balance();
    };
}

#endif