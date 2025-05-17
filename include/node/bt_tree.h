#ifndef __BWT_H__
#define __BWT_H__
#include <cstring>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include <mutex>
#include <functional>
#include "bw_node.h"
#include "binary.h"
#include "util/secure_ptr.h"
#include "util/memory_pool.h"
#include "util/thread_pool.h"
#include "util/safe_vector.h"
#include "util/log.h"
#include "util/random.h"
#include "util/cell.h"
#include "util/safe_queue.h"
#include "util/token.h"
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

    struct binary_node_info{
        Binary data;
        size_t bitmap;
    };

    typedef file_data TreeNode;

    typedef white_node_info WhiteNodeInfo;

    typedef binary_node_info BinaryNodeInfo;

    constexpr size_t SIZE_OF_NODE_DATA = BwtFS::BLOCK_SIZE - sizeof(uint8_t);

    /*
    * TODO: 
    *     1. 获取全局的文件系统对象
    *     2. 接收来自tree的黑白节点数据
    *        （只需要将二进制数据写入文件，
    *            不需要关系内容是什么，
    *            节点是什么）
    *     3. 将黑白节点数据写入文件系统  
    *     4. 若文件系统写入失败，则回滚
    *     5. 若文件系统写入成功，则提交
    *     
    *     每个tree对象都绑定一个事务对象
    */
    class TransactionWriter{
        public:
            TransactionWriter(){
                m_fs = BwtFS::System::getBwtFS();
            }
            TransactionWriter(const TransactionWriter&) = delete;
            TransactionWriter& operator=(const TransactionWriter&) = delete;
            TransactionWriter(TransactionWriter&&) = delete;
            TransactionWriter& operator=(TransactionWriter&&) = delete;
            ~TransactionWriter() = default;

            /*
            * 写入数据
            * 返回写入的块的索引
            */
            size_t write(Binary& data){
                BinaryNodeInfo info;
                info.data = data;
                auto t = m_fs->bitmap->getFreeBlock();
                if (t == 0){
                    LOG_ERROR << "No free block";
                    throw std::runtime_error("No free block");
                }
                info.bitmap = t;
                m_data_queue.enqueue(info);
                return t;
            }
            void commit(){
                // 提交事务的逻辑
                while(!m_size_queue.empty()){
                    size_t bitmap;
                    m_size_queue.dequeue(bitmap);
                    m_fs->bitmap->set(bitmap);
                    // LOG_DEBUG << "Set bitmap: " << bitmap;
                }
                
            }
            void set_write_finished(bool finished){
                m_write_finished = finished;
            }
            void write_fs(){
                while(!m_data_queue.empty() || !m_write_finished){
                    if (m_data_queue.empty()){
                        continue;
                    }
                    BinaryNodeInfo data;
                    m_data_queue.dequeue(data);
                    // LOG_INFO << "Bitmap: " << data.bitmap;
                    // LOG_INFO << "Data: " << data.data.size();

                    m_fs->write(data.bitmap, data.data);
                    m_size_queue.enqueue(data.bitmap);
                }
            }
        private:
            std::shared_ptr<BwtFS::System::FileSystem> m_fs;
            safe_queue<BinaryNodeInfo> m_data_queue;
            safe_queue<size_t> m_size_queue;
            bool m_write_finished = false;
    };

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
                m_thread_pool.submit([this]{
                    this->generate_tree();
                });
                m_thread_pool.submit([this]{
                    this->m_transaction_writer.write_fs();
                });
            };
            bw_tree(const std::string & token){
                m_token = token;
                Token t(token);
            }

            bw_tree(const bw_tree&) = delete;
            bw_tree& operator=(const bw_tree&) = delete;
            bw_tree(bw_tree&&) = delete;
            bw_tree& operator=(bw_tree&&) = delete;
            ~bw_tree() = default;
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
                set_write_finished(true);
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
            * 新建类，用于管理位图数据和将数据写入文件系统
            * 同时应加入事务机制，能够在出现意外的时候回滚
            * 
            * 数据写入文件系统的逻辑：
            *   1. 读取数据，数据满4096字节（若文件结束，可不满4096字节），将数据节点写入内存池，并将指针加入队列
            *   2. 从队列中取出指针，转换为白节点
            *   3. 将白节点加入黑白树的队列
            *   4. 将白节点加入黑白树
            *   5. 根据白节点的信息生成entry，然后将entry加入黑节点
            *   6. 将白节点加入队列，待写入文件系统
            *   7. 若黑节点写满4096字节，则将黑节点写入文件系统，并生成新的entry
            *   8. 重复步骤2-6，直到文件结束
            *   9. 文件完全写入之后，生成token
            */
            std::string generate_tree(){
                int bkn_index = 0;
                black_node<RCAEncryptor>* bkn = new black_node<RCAEncryptor>(bkn_index++);
                constexpr int entry_num = BwtFS::BLOCK_SIZE / SIZE_OF_ENTRY;
                auto seeds = BwtFS::Util::RandNumbers(entry_num, std::hash<BwtFS::Node::black_node<RCAEncryptor>*>{}(bkn), 0, 1 << 30);
                auto levels = BwtFS::Util::RandNumbers(entry_num, std::hash<std::vector<int>*>{}(&seeds), 0, 1 << 8);
                int seed, level;
                while(!is_write_finished() || !m_nodes.empty()){
                    while(!m_nodes.empty()){
                        auto index = bkn->size();
                        seed = seeds.back();
                        level = levels.back();
                        seeds.pop_back();
                        levels.pop_back();
                        if (seeds.empty()){
                            seeds = BwtFS::Util::RandNumbers(entry_num, std::hash<BwtFS::Node::black_node<RCAEncryptor>*>{}(bkn), 0, 1 << 30);
                            levels = BwtFS::Util::RandNumbers(entry_num, std::hash<std::vector<int>*>{}(&seeds), 0, 1 << 8);
                        }
                        auto node = get_node(index, seed, level);
                        auto bitmap = m_transaction_writer.write(node.data);
                        auto entry = generate_entry(bitmap, node.start, node.length, seed, level, false);
                        bkn->add_entry(entry);
                        if (bkn->is_fill()){
                            seed = seeds.back();
                            level = levels.back();
                            levels.pop_back();
                            seeds.pop_back();
                            if (seeds.empty()){
                                seeds = BwtFS::Util::RandNumbers(entry_num, std::hash<BwtFS::Node::black_node<RCAEncryptor>*>{}(bkn), 0, 1 << 30);
                                levels = BwtFS::Util::RandNumbers(entry_num, std::hash<std::vector<int>*>{}(&seeds), 0, 1 << 8);
                            }
                            auto binary_data = bkn->to_binary(seed, level);
                            bitmap = m_transaction_writer.write(binary_data);
                            entry = generate_entry(bitmap, bkn->get_start(), bkn->get_length(), seed, level, true);
                            delete bkn;
                            bkn = new black_node<RCAEncryptor>(bkn_index++);
                            bkn->add_entry(entry);
                        }
                        // LOG_INFO << " nodes size: " << m_nodes.size();
                    }
                    // LOG_DEBUG << "Black node size: " << bkn->size() << 
                    //         " is_write_finished: " << is_write_finished() <<
                    //         " nodes size: " << m_nodes.size();
                }
                auto binary_data = bkn->to_binary(seed, level);
                auto bitmap = m_transaction_writer.write(binary_data);
                this->m_transaction_writer.set_write_finished(true);
                m_token = generate_token(bitmap, bkn->get_start(), bkn->get_length(), seed, level);
                is_generate = true;
                return m_token;
            }

            bool is_generate_tree(){
                return is_generate;
            }

            void join(){
                while(!is_generate){
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }

            std::string get_token(){
                return m_token;
            }

        private:
            MemoryPool<TreeNode>& m_memory_pool = get_pool<TreeNode>(BwtFS::SIZE::__MEMORY_POOL_INIT_SIZE);
            safe_queue<TreeNode*> m_nodes;
            ThreadPool m_thread_pool = ThreadPool(BwtFS::SIZE::__THREAD_POOL_SIZE);
            TreeNode* m_cache_data = nullptr;
            safe_vector<black_node<RCAEncryptor>*> m_black_nodes;
            bool write_finished = false;
            bool is_generate = false;
            std::mutex m_write_finish_mutex;
            TransactionWriter m_transaction_writer;
            std::string m_token;
            

            /*
            * 生成黑白树节点
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
                m_memory_pool.destroy(node);
                return {binary_data, wn.get_start(), wn.get_length()};
            }
            WhiteNodeInfo get_node(int index, unsigned seed, uint8_t level){
                if (m_nodes.empty()){
                    LOG_ERROR << "No available nodes in the pool";
                    return {Binary(), 0, 0};
                }
                TreeNode* node;
                m_nodes.dequeue(node);
                auto wnb = Binary(reinterpret_cast<std::byte*>(node->data), node->size);
                auto wn = white_node<RCAEncryptor>(wnb, index);
                auto binary_data = wn.to_binary(seed, level);
                m_memory_pool.destroy(node);
                return {binary_data, wn.get_start(), wn.get_length()};
            }
            /*
            * 生成entry
            */
            entry generate_entry(size_t bitmap, uint16_t start, uint16_t length, 
                                uint16_t seed, uint8_t level, bool is_black){
                if (is_black){
                    // 生成黑节点的entry
                    return entry(bitmap, NodeType::BLACK_NODE, start, length, seed, level);
                }
                // 生成entry的逻辑
                return entry(bitmap, NodeType::WHITE_NODE, start, length, seed, level);
            }
            TreeNode* get_node(){
                this->m_cache_data = m_memory_pool.create();
                this->m_cache_data->size = 0;
                return this->m_cache_data;
            }
            void set_write_finished(bool finished){
                std::unique_lock<std::mutex> lock(m_write_finish_mutex);
                this->write_finished = finished;
            }
            bool is_write_finished(){
                std::unique_lock<std::mutex> lock(m_write_finish_mutex);
                return this->write_finished;
            }
            std::string generate_token(size_t bitmap, unsigned start, unsigned length, unsigned seed, uint8_t level){
                m_transaction_writer.commit();
                // 生成token的逻辑
                Token token(bitmap, start, length, seed, level);
                return token.generate_token();
            }
    };
}
#endif // __BWT_H__