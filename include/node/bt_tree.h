#ifndef __BWT_H__
#define __BWT_H__
#include <cstring>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include "bw_node.h"
#include "binary.h"
#include "util/secure_ptr.h"
#include "util/memory_pool.h"
#include "util/thread_pool.h"
#include "util/log.h"
#include "util/random.h"
#include "util/cell.h"
#include "util/safe_queue.h"
#include "file/system.h"
#include "entry.h"
#include "config.h"
#include "bw_node.h"
#include "entry.h"
namespace BwtFS::Node{

    struct file_data{
        std::byte data[BwtFS::BLOCK_SIZE - sizeof(uint8_t)];
        uint16_t size;
    };

    struct white_node_info{
        Binary data;
        unsigned start;
        unsigned length;
    };

    typedef file_data TreeNode;

    typedef white_node_info WhiteNodeInfo;

    constexpr size_t SIZE_OF_NODE_DATA = BwtFS::BLOCK_SIZE - sizeof(uint8_t);


    /*
    * 黑白树
    * 采用内存池来分配内存
    * 每个节点的大小为4096字节
    * 向黑白树写入数据：
    *       1. 读取数据，数据满4096字节（若文件结束，可不满4096字节），将数据节点写入内存池，并将指针加入队列
    *       2. 从队列中取出指针，转换为白节点
    *       3. 将白节点加入黑白树
    *       4. 根据白节点的信息生成entry，然后将entry加入黑节点
    *       5. 将白节点写入文件系统
    *       6. 若黑节点写满4096字节，则将黑节点写入文件系统，并生成新的entry
    *       7. 重复步骤2-6，直到文件结束
    *       8. 文件完全写入之后，生成token
    * 
    * 
    * 
    *   从黑白树读取数据：
    *       从黑白树读取文件首先需要打开文件
    *       1. 遍历黑白树，读取黑节点的entry
    *       2. 根据entry的信息生成索引列表（在secure_ptr中进行）
    *    根据索引列表读取数据
    * 
    * 
    * 黑白树需要确保：只能同时有一个线程在写入数据，写入失败时需要回滚
    */
    class bw_tree{
        public:
            bw_tree(){
            }
            bw_tree(const bw_tree&) = delete;
            bw_tree& operator=(const bw_tree&) = delete;
            bw_tree(bw_tree&&) = delete;
            bw_tree& operator=(bw_tree&&) = delete;
            ~bw_tree() = default;
            /*
            * 生成节点
            * 生成节点的数据需要为4096字节
            * 这里使用内存池来分配内存
            * 文件数据生成的节点为白节点
            * 返回白节点的二进制数据
            */
            WhiteNodeInfo get_node(int index){
                if (m_nodes.empty()){
                    LOG_ERROR << "No available nodes in the pool";
                    return {Binary(), 0, 0};
                }
                TreeNode* node;
                m_nodes.dequeue(node);
                auto wnb = Binary(reinterpret_cast<std::byte*>(node->data), node->size);
                auto wn = white_node<void>(wnb, index);
                auto binary_data = wn.to_binary();
                m_pool.destroy(node);
                return {binary_data, wn.get_start(), wn.get_length()};
            }
            /*
            * 将数据写入黑白树
            */
            void write(char* data, size_t size){
                if (this->m_cache_data == nullptr){
                    this->get_node();
                }
                while(size){
                    size_t copy_size = std::min(size, SIZE_OF_NODE_DATA - m_cache_data->size);
                    std::memcpy(m_cache_data->data + m_cache_data->size, data, copy_size);
                    size -= copy_size;
                    m_cache_data->size += copy_size;
                    if (m_cache_data->size == SIZE_OF_NODE_DATA){
                        m_nodes.enqueue(m_cache_data);
                        this->get_node();
                    }
                }
            }
            /*
            * 将缓存的数据写入黑白树
            */
            void flush(){
                if (m_cache_data != nullptr && m_cache_data->size > 0){
                    m_nodes.enqueue(m_cache_data);
                }
            }
            /*
            * 从黑白树中读取数据
            */
            void read(char* data, size_t size){
                // 读取数据的逻辑
            }

            int get_node_count(){
                return m_nodes.size();
            }
            /*
            * 生成entry
            */
            entry generate_entry(int index, unsigned start, unsigned length, unsigned seed, uint8_t level, bool is_black){
                if (is_black){
                    // 生成黑节点的entry
                    return entry(index, NodeType::BLACK_NODE, start, length, seed, level);
                }
                // 生成entry的逻辑
                return entry(index, NodeType::WHITE_NODE, start, length, seed, level);
            }

            private:
                // 内存池
                MemoryPool<TreeNode>& m_pool = get_pool<TreeNode>(BwtFS::SIZE::__MEMORY_POOL_INIT_SIZE);
                safe_queue<TreeNode*> m_nodes;
                ThreadPool m_thread_pool = ThreadPool(BwtFS::SIZE::__THREAD_POOL_SIZE);
                TreeNode* m_cache_data = nullptr;
                black_node<RCAEncryptor>* m_black_node = nullptr;
                
                TreeNode* get_node(){
                    this->m_cache_data = m_pool.create();
                    this->m_cache_data->size = 0;
                    return this->m_cache_data;
                }
                void release_node(){
                    if (this->m_cache_data != nullptr){
                        m_pool.destroy(this->m_cache_data);
                        this->m_cache_data = nullptr;
                    }
                }
            };
    
}



#endif // __BWT_H__