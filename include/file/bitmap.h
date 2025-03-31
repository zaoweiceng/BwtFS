#ifndef BITMAP_H
#define BITMAP_H
#include <cstddef>
#include <string>
#include <vector>
#include "node/binary.h"
#include "file/system_file.h"
namespace BwtFS::System{
    class Bitmap{
        /*
        * 位图类
        * 用于位图的读写操作
        * @author: zaoweiceng
        * @data: 2025-03-30
        */
        public:
            Bitmap();
            Bitmap(const Bitmap& other) = delete;
            Bitmap& operator=(const Bitmap& other) = delete;
            Bitmap(Bitmap&& other) = delete;
            ~Bitmap() = default;

            void set(const size_t index, const bool value);
            bool get(const size_t index) const;
            void write(const BwtFS::Node::Binary data);
            
            void init(const size_t size);


        private:
            BwtFS::Node::Binary bitmap;
            BwtFS::Node::Binary bitmap_wear;
            size_t size;
            size_t size_wear;

            BwtFS::System::File file;
    };
}

#endif