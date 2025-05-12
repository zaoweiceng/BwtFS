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
#include <algorithm>
#include <utility>
#include "config.h"
constexpr size_t initialSize = BwtFS::SIZE::__MEMORY_POOL_INIT_SIZE; // 内存池初始大小
class MemoryPoolBase {
protected:
    // 链表节点结构
    struct FreeNode {
        FreeNode* next;
    };

    // 内存块结构
    struct MemoryChunk {
        char* memory; // 内存块起始地址
        size_t size;  // 内存块大小
    };

    MemoryPoolBase(size_t blockSize, size_t initialSize)
        : blockSize_(std::max(blockSize, sizeof(FreeNode))),
          initSize(initialSize * 2),
          totalBlocks_(0) {
        expandPool(initialSize);
    }

    virtual ~MemoryPoolBase() {
        for (auto& chunk : chunks_) {
            delete[] chunk.memory;
        }
    }

    void* allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!freeList_) {
            // size_t expandSize = std::max(initSize, static_cast<size_t>(1));
            expandPool(initSize);
        }
        
        void* result = freeList_;
        freeList_ = freeList_->next;
        --freeBlocks_;
        
        return result;
    }

    void deallocate(void* ptr) {
        if (!ptr) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        FreeNode* node = static_cast<FreeNode*>(ptr);
        node->next = freeList_;
        freeList_ = node;
        ++freeBlocks_;
        
        if (freeBlocks_ > initSize*2) {
            shrinkPool();
        }
    }

    size_t blockSize() const { return blockSize_; }

private:
    void expandPool(size_t size) {
        char* memory = new char[blockSize_ * size];
        chunks_.push_back({memory, size});
        
        for (size_t i = 0; i < size; ++i) {
            FreeNode* node = reinterpret_cast<FreeNode*>(memory + i * blockSize_);
            node->next = freeList_;
            freeList_ = node;
        }
        
        freeBlocks_ += size;
        totalBlocks_ += size;
        // std::cout << "Expanded memory pool, total blocks: " 
        //           << totalBlocks_ << ", free blocks: " << freeBlocks_ << std::endl;
    }

    void shrinkPool() {
        // 1. 计算需要保留的最小空闲块数（当前总块数的一半或至少1个）
        size_t minFreeBlocksToKeep = std::max(totalBlocks_ / 2, static_cast<size_t>(1));
        
        // 2. 如果没有多余的空闲块需要释放，直接返回
        if (freeBlocks_ <= minFreeBlocksToKeep) {
            return;
        }

        // 3. 计算可以释放的多余空闲块数
        size_t blocksCanFree = freeBlocks_ - minFreeBlocksToKeep;
        size_t blocksFreed = 0;
        
        // 4. 从最早分配的内存块开始检查（FIFO顺序）
        auto it = chunks_.begin();
        while (it != chunks_.end() && blocksFreed < blocksCanFree) {
            MemoryChunk& chunk = *it;
            size_t blocksInChunk = 0;
            
            // 5. 检查这个chunk中有多少块在空闲列表中
            FreeNode** nodePtr = &freeList_;
            while (*nodePtr) {
                char* nodeMem = reinterpret_cast<char*>(*nodePtr);
                if (nodeMem >= chunk.memory && 
                    nodeMem < chunk.memory + blockSize_ * chunk.size) {
                    // 这个块在空闲列表中，可以释放
                    *nodePtr = (*nodePtr)->next;
                    ++blocksInChunk;
                } else {
                    nodePtr = &(*nodePtr)->next;
                }
            }

            // 6. 如果这个chunk的所有块都在空闲列表中，可以安全释放整个chunk
            if (blocksInChunk == chunk.size) {
                delete[] chunk.memory;
                blocksFreed += blocksInChunk;
                totalBlocks_ -= chunk.size;
                freeBlocks_ -= blocksInChunk;
                
                // 从chunks_列表中移除
                it = chunks_.erase(it);
                
                // std::cout << "Freed entire chunk: " << chunk.size 
                //         << " blocks. Total freed: " << blocksFreed 
                //         << "/" << blocksCanFree << std::endl;
            } else {
                // 这个chunk中有块正在使用，跳过
                ++it;
            }
        }
        
        // std::cout << "Shrinking completed. Freed " << blocksFreed 
        //         << " blocks. Remaining - Total: " << totalBlocks_ 
        //         << ", Free: " << freeBlocks_ << std::endl;
    }

protected:
    FreeNode* freeList_ = nullptr;
    size_t blockSize_;
    size_t freeBlocks_ = 0;
    size_t totalBlocks_ = 0;
    size_t initSize;
    std::vector<MemoryChunk> chunks_;
    std::mutex mutex_;
};

// 模板化内存池
template <typename T>
class MemoryPool : private MemoryPoolBase {
public:
    explicit MemoryPool(size_t initial = initialSize)
        : MemoryPoolBase(sizeof(T), initial) {}

    // 分配内存并构造对象
    template <typename... Args>
    T* create(Args&&... args) {
        void* mem = allocate();
        return new (mem) T(std::forward<Args>(args)...);
    }

    // 销毁对象并释放内存
    void destroy(T* ptr) {
        if (ptr != nullptr) {
            ptr->~T();
            deallocate(ptr);
        }
    }
};

// 创建内存池的辅助函数
template <typename T, typename... Args>
MemoryPool<T>& make_pool(Args&&... args) {
    static MemoryPool<T> pool(std::forward<Args>(args)...);
    return pool;
}

template <typename T, typename... Args>
MemoryPool<T>& get_pool(Args&&... args) {
    static bool initialized = false;
    static MemoryPool<T>* pool = nullptr;
    // std::cout << "get_pool" << std::endl;
    if (!initialized) {
        // std::cout << "Initializing memory pool" << std::endl;
        pool = &make_pool<T>(std::forward<Args>(args)...);
        initialized = true;
    }

    return *pool;
}

template <typename T>
class pool_ptr{
    public:
        template <typename... Args>
        explicit pool_ptr(Args&&... args) {
            this->pool_ = &get_pool<T>(initialSize);
            this->ptr_ = this->pool_->create(std::forward<Args>(args)...);
        }
        // pool_ptr(const pool_ptr&) = delete;
        // pool_ptr& operator=(const pool_ptr&) = delete;
        pool_ptr(pool_ptr&& other) noexcept : ptr_(other.ptr_), pool_(other.pool_) {
            other.ptr_ = nullptr;
            other.pool_ = nullptr;
        }
        pool_ptr& operator=(pool_ptr&& other) noexcept {
            if (this != &other) {
                ptr_ = other.ptr_;
                pool_ = other.pool_;
                other.ptr_ = nullptr;
                other.pool_ = nullptr;
            }
            return *this;
        }
        ~pool_ptr() {
            if (ptr_ != nullptr && pool_ != nullptr) {
                // std::cout << "pool_ptr destructor" << std::endl;
                this->pool_->destroy(ptr_);
            }
        }
        void destroy() {
            if (ptr_ != nullptr && pool_ != nullptr) {
                // std::cout << "pool_ptr destroy" << std::endl;
                this->pool_->destroy(ptr_);
                ptr_ = nullptr;
            }
        }

        T* operator->() {
            if (ptr_ == nullptr) {
                // std::cerr << "Error: Attempt to access a null pointer." << std::endl;
                return nullptr;
            }
            return ptr_;
        }
    
    private:
        T* ptr_;
        MemoryPool<T>* pool_;
};

template <typename T, typename... Args>
auto make_pool_ptr(Args&&... args) -> pool_ptr<T> {
    return pool_ptr<T>(std::forward<Args>(args)...);
}
#endif // __MEMORY_POOL_H