#ifndef __BWN_H__
#define __BWN_H__
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <utility>
#include <memory>
#include <cstddef>      
#include <new>         
#include <cstdlib>   
#include <climits>     
#include <iostream> 
#include "binary.h"
#include "util/secure_ptr.h"
#include "util/random.h"
#include "util/cell.h"
#include "entry.h"
#include "config.h"
#include "util/log.h"

using BwtFS::Util::Logger;

namespace BwtFS::Node{

    template<typename E = Encryptor>
    class tree_base_node{
        public:
            // 加密的构造函数
            tree_base_node(Binary value, uint8_t level, unsigned seed, unsigned start, unsigned length)
             : m_value(value), start(start), length(length){
                E e;
                if constexpr (E::value == "RCAEncryptor") {
                    auto seeds = BwtFS::Util::RandNumbers<uint16_t>(level, seed, 0, 1<<15);
                    std::reverse(seeds.begin(), seeds.end());
                    for (int i = 0; i < level; i++) {
                        E e;
                        e.setBinary(m_value);
                        e.setSeed(seeds[i]);
                        e.decrypt(m_value.data(), m_value.size());
                    }
                }
                e.decrypt(m_value.data(), m_value.size());
             }
            tree_base_node() = default;
        protected:
            Binary m_value; // 节点内容
            uint8_t index; // 节点索引
            unsigned start; // 节点起始位置
            unsigned length; // 节点长度

    };

    template<>
    class tree_base_node<void>{
        public:
            // 未加密数据的构造函数
            tree_base_node(Binary value, unsigned start, unsigned length)
             : m_value(value), start(start), length(length) {}
            tree_base_node() = default;
        protected:
            Binary m_value; // 节点内容
            uint8_t index; // 节点索引
            uint16_t start; // 节点起始位置
            uint16_t length; // 节点长度
    };

    template<typename E>
    class white_node : public tree_base_node<E>{
        public:

            white_node(Binary value, uint8_t level, uint16_t seed, uint16_t start, uint16_t length)
             : tree_base_node<E>(value, level, seed, start, length) {
                this->index = *reinterpret_cast<uint8_t*>(value.read(0, sizeof(uint8_t)).data());
                this->m_value = Binary(value.read(start, length));
             }

            white_node(Binary value, uint16_t start, uint16_t length) {
                // LOG_DEBUG << "start: " << start << ", length: " << length;
                this->index = *reinterpret_cast<uint8_t*>(value.read(0, sizeof(uint8_t)).data());
                // LOG_DEBUG << "index: " << this->index;
                this->m_value = Binary(value.read(start, length));
             }

            white_node(Binary& data, uint8_t index){
                this->index = index;
                this->m_value = data;
                this->length = this->m_value.size();
                this->start = 0;
            }

            white_node() = delete;

            Binary data() const { return this->m_value; }

            Binary to_binary() {
                Binary binary_data;
                binary_data.append(sizeof(uint8_t), reinterpret_cast<std::byte*>(&this->index));
                unsigned gap = BwtFS::BLOCK_SIZE - sizeof(uint8_t) - this->m_value.size();
                int rand = BwtFS::Util::RandNumber(std::time(nullptr), 0, gap);
                binary_data += Binary(BwtFS::Util::RandBytes(rand, std::time(nullptr), 0, 255));
                this->start = binary_data.size();
                binary_data += std::move(this->m_value);
                binary_data += Binary(BwtFS::Util::RandBytes(BwtFS::BLOCK_SIZE - binary_data.size(), std::time(nullptr), 0, 255));
                return binary_data;
            }

            Binary to_binary(uint16_t seed, uint8_t level) {
                Binary binary_data = this->to_binary();
                auto seeds = BwtFS::Util::RandNumbers<uint16_t>(level, seed, 0, 1<<15);
                if constexpr (E::value == "RCAEncryptor") {
                    for (int i = 0; i < level; i++) {
                        E e;
                        e.setBinary(binary_data);
                        e.setSeed(seeds[i]);
                        e.encrypt(binary_data.data(), binary_data.size());
                    }
                }else{
                    E e;
                    e.encrypt(binary_data.data(), binary_data.size());
                }
                return binary_data;
            }

            void set_data(Binary value) {
                this->m_value.write(value);
                this->length = value.size();
            }

            void append_data(Binary value) {
                this->m_value.append(value);
                this->length += value.size();
            }

            uint8_t get_index() const {
                return this->index;
            }

            uint16_t get_start() const {
                return this->start;
            }

            uint16_t get_length() const {
                return this->length;
            }

            void set_index(uint8_t index) {
                this->index = index;
            }
    };

    template<typename E>
    class black_node : public tree_base_node<E>{
        public:
        
            black_node(Binary value, uint8_t level, uint16_t seed, uint16_t start, uint16_t length)
             : tree_base_node<E>(value, level, seed, start, length), m_entry_list(make_secure<entry_list>()) {
                this->index = *reinterpret_cast<uint8_t*>(value.read(0, sizeof(uint8_t)).data());
                this->m_value = Binary(value.read(start, length));
                auto entry_data = entry_list::from_binary(this->m_value, length/entry::size());
                for (int i = 0; i < entry_data.size(); i++) {
                    this->m_entry_list->add_entry(entry_data.get_entry(i));
                }
            }

            black_node(Binary value, uint16_t start, uint16_t length)
             : m_entry_list(make_secure<entry_list>()) {
                this->length = 0;
                this->index = *reinterpret_cast<uint8_t*>(value.read(0, sizeof(uint8_t)).data());
                this->m_value = Binary(value.read(start, length));
                // LOG_DEBUG << "start: " << start << ", length: " << length << ", value: " << length/entry::size();
                auto entry_data = entry_list::from_binary(this->m_value, length/entry::size());
                for (int i = 0; i < entry_data.size(); i++) {
                    this->m_entry_list->add_entry(entry_data.get_entry(i));
                }
            }

            black_node(Binary& data, uint8_t index) : m_entry_list(make_secure<entry_list>()){
                this->index = index;
                this->m_value = data;
                this->length = this->m_value.size();
                this->start = 0;
                auto entry_data = entry_list::from_binary(this->m_value, this->length/entry::size());
                for (int i = 0; i < entry_data.size(); i++) {
                    this->m_entry_list->add_entry(entry_data.get_entry(i));
                }
            }
            
            black_node() = delete;

            black_node(uint8_t index): m_entry_list(make_secure<entry_list>()) {
                this->length = 0;
                this->index = index;
            };

            Binary data() const { return this->m_value; }

            Binary to_binary() {
                Binary binary_data;
                binary_data.append(sizeof(uint8_t), reinterpret_cast<std::byte*>(&this->index));
                m_entry_list->shuffle();
                Binary entry_data = m_entry_list->to_binary();
                unsigned gap = BwtFS::BLOCK_SIZE - sizeof(uint8_t) - entry_data.size();
                int rand = BwtFS::Util::RandNumber(std::time(nullptr), 0, gap);
                binary_data += Binary(BwtFS::Util::RandBytes(rand, std::time(nullptr), 0, 255));
                this->start = binary_data.size();
                binary_data += std::move(entry_data);
                binary_data += Binary(BwtFS::Util::RandBytes(BwtFS::BLOCK_SIZE - binary_data.size(), std::time(nullptr), 0, 255));
                return binary_data;
            }

            Binary to_binary(uint16_t seed, uint8_t level) {
                Binary binary_data = this->to_binary();
                auto seeds = BwtFS::Util::RandNumbers<uint16_t>(level, seed, 0, 1<<15);
                if constexpr (E::value == "RCAEncryptor") {
                    for (int i = 0; i < level; i++) {
                        E e;
                        e.setBinary(binary_data);
                        e.setSeed(seeds[i]);
                        e.encrypt(binary_data.data(), binary_data.size());
                    }
                }else{
                    E e;
                    e.encrypt(binary_data.data(), binary_data.size());
                }
                return binary_data;
            }

            uint8_t get_index() const {
                return this->index;
            }

            unsigned get_start() const {
                return this->start;
            }

            unsigned get_length() const {
                return this->length;
            }

            void set_index(uint8_t index) {
                this->index = index;
            }

            void add_entry(const entry& e) {
                m_entry_list->add_entry(e);
                this->length += entry::size();
            }

            bool is_fill() {
                return m_entry_list->is_fill();
            }

            size_t size() {
                return m_entry_list->size();
            }

            entry get_entry(size_t index) {
                BwtFS::Node::entry e = m_entry_list->get_entry(index);
                return e;
            }

        private:
            secure_ptr<BwtFS::Node::entry_list> m_entry_list;

    };

    // 打开文件时使用secure_ptr
}
#endif // __BWN_H__