#ifndef ENTRY_H
#define ENTRY_H
#include "node/binary.h"
#include <algorithm>
#include <vector>
#include <random>
#include "util/log.h"

using BwtFS::Util::Logger;

namespace BwtFS::Node{
    enum class NodeType{
        WHITE_NODE = 0, // 白节点
        BLACK_NODE = 1, // 黑节点
    };

    // entry 定义：bitmap位置、节点类型、起始位置、长度、随机种子
    class entry {
        private:
            size_t   bitmap;        // 位图
            NodeType type;          // 节点类型
            uint16_t start;         // 起始位置
            uint16_t length;        // 长度
            uint16_t seed;          // 随机数种子
            uint8_t  level;         // 节点的层级, 如果为0，则表示没有加密

        public:
            entry(size_t bitmap, NodeType type, uint16_t start, uint16_t length, uint16_t seed = 0, uint8_t level = 0)
            : bitmap(bitmap), type(type), start(start), length(length), seed(seed), level(level) {}
            entry(const entry&) = default;
            inline size_t get_bitmap() const { return bitmap; }
            inline NodeType get_type() const { return type; }
            inline uint16_t get_start() const { return start; }
            inline uint16_t get_length() const { return length; }
            inline uint16_t get_seed() const { return seed; }
            inline uint8_t get_level() const { return level; }
            Binary to_binary();
            static entry from_binary(Binary& binary_data);
            static inline size_t size() {
                return sizeof(size_t) + sizeof(bool) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t);
            }
    };


    class entry_list{
        public:
            entry_list(const entry_list&){
                this->entries = std::vector<entry>();
            }
            entry_list(entry_list&&) = default;
            entry_list& operator=(const entry_list&) = default;
            entry_list& operator=(entry_list&&) = default;
            entry_list(std::vector<entry>&& entries) : entries(std::move(entries)) {}
            entry_list(const std::vector<entry>& entries) : entries(entries) {};
            entry_list() : entries() {}

            inline void add_entry(const entry& e) {
                entries.push_back(e);
            }

            inline void add_entry(entry&& e) {
                entries.push_back(std::move(e));
            }

            inline entry get_entry(size_t index) {
                if (index >= entries.size()) {
                    throw std::out_of_range("Index out of range");
                }
                return entries[index];
            }

            inline size_t size() const {
                return entries.size();
            }

            static entry_list from_binary(Binary& binary_data, int num_entries);

            Binary to_binary();

            void shuffle() {
                std::shuffle(entries.begin(), entries.end(), std::mt19937(std::random_device()()));
            }

        private:
            std::vector<entry> entries;

    };
}

#endif