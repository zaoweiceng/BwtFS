#ifndef __MEMORY_POOL_H
#define __MEMORY_POOL_H
#include <vector>
#include <queue>
#include <thread>
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
template <typename T>
class memory_pool {
    private:
        struct memory_block {
            memory_block* next_block;  
            char* data;               
            
            memory_block(memory_block* next = nullptr) 
                : next_block(next), data(nullptr) {}
        };

        union free_node {
            T element;              
            free_node* next_node;    
            
            free_node(free_node* next = nullptr) : next_node(next) {}
        };

    public:
        typedef T               value_type;
        typedef T*              pointer;
        typedef const T*        const_pointer;
        typedef T&              reference;
        typedef const T&        const_reference;
        typedef size_t          size_type;
        typedef ptrdiff_t       difference_type;

        explicit memory_pool(size_type chunk_size = 32) 
            : free_slots_(nullptr), 
            current_block_(nullptr),
            chunk_size_(chunk_size) {
            // 确保chunk_size至少为1
            if (chunk_size == 0) {
                chunk_size_ = 1;
            }
        }

        ~memory_pool() {
            while (current_block_ != nullptr) {
                memory_block* next = current_block_->next_block;
                operator delete(current_block_->data);
                delete current_block_;
                current_block_ = next;
            }
        }

        // 分配一个对象的内存 (不构造对象)
        pointer allocate() {
            // 如果有空闲槽，直接使用
            if (free_slots_ != nullptr) {
                pointer result = reinterpret_cast<pointer>(free_slots_);
                free_slots_ = free_slots_->next_node;
                return result;
            }
            
            // 没有空闲槽，需要分配新的内存块
            if (current_block_ == nullptr || free_slots_ == nullptr) {
                allocate_block();
            }
            
            // 从新分配的内存块中返回一个槽
            pointer result = reinterpret_cast<pointer>(free_slots_);
            free_slots_ = free_slots_->next_node;
            return result;
        }

        // 释放一个对象的内存 (不析构对象)
        void deallocate(pointer p) {
            if (p != nullptr) {
                // 将释放的内存转换为空闲节点并加入空闲链表
                reinterpret_cast<free_node*>(p)->next_node = free_slots_;
                free_slots_ = reinterpret_cast<free_node*>(p);
            }
        }

        void construct(pointer p, const_reference val) {
            // 使用placement new在指定内存构造对象
            new (p) value_type(val);
        }

        void destroy(pointer p) {
            if (p != nullptr) {
                // 显式调用析构函数
                p->~value_type();
            }
        }

        // 返回对象大小
        size_type object_size() const {
            return sizeof(value_type);
        }

    private:
        // 分配一个新的内存块
        void allocate_block() {
            // 计算需要分配的总字节数
            size_type block_size = chunk_size_ * sizeof(value_type);
            
            // 分配内存块头和新内存区域
            char* new_memory = static_cast<char*>(operator new(block_size));
            memory_block* new_block = new memory_block(current_block_);
            
            // 设置新内存块的数据指针
            new_block->data = new_memory;
            current_block_ = new_block;
            
            // 将新内存块划分为空闲节点并加入空闲链表
            char* p = new_memory;
            for (size_type i = 0; i < chunk_size_ - 1; ++i) {
                reinterpret_cast<free_node*>(p)->next_node = 
                    reinterpret_cast<free_node*>(p + sizeof(value_type));
                p += sizeof(value_type);
            }
            
            // 最后一个节点的next设为nullptr
            reinterpret_cast<free_node*>(p)->next_node = nullptr;
            free_slots_ = reinterpret_cast<free_node*>(new_memory);
        }

        free_node* free_slots_;      // 空闲节点链表
        memory_block* current_block_; // 当前内存块
        size_type chunk_size_;       // 每次分配的块大小(对象数量)
        
        memory_pool(const memory_pool&) = delete;
        memory_pool& operator=(const memory_pool&) = delete;
};

#endif // __MEMORY_POOL_H