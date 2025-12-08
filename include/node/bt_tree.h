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

    struct visit_node{
        size_t bitmap;
        uint16_t start;
        uint16_t length;
        uint16_t seed;
        uint8_t level;
    };

    struct visit_node_list{
        std::vector<visit_node> nodes;
        size_t size;
    };

    typedef file_data TreeNode;

    typedef white_node_info WhiteNodeInfo;

    typedef binary_node_info BinaryNodeInfo;

    typedef visit_node VisitNode;

    typedef visit_node_list VisitNodeList;

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

    class TreeDataReader{
        public:
            TreeDataReader(const std::string& token, bool is_delete = false){
                Token t(token); 
                m_fs = BwtFS::System::getBwtFS();
                if(is_delete){
                    delete_bitmap.push_back(t.get_bitmap());
                }
                LOG_DEBUG << "Bitmap: " << t.get_bitmap();

                m_entry_queue.emplace(t.get_bitmap(), NodeType::BLACK_NODE, t.get_start(), 
                                        t.get_length(), t.get_seed(), t.get_level());
                init(is_delete);
            }
            TreeDataReader(const TreeDataReader&) = delete;
            TreeDataReader& operator=(const TreeDataReader&) = delete;
            TreeDataReader(TreeDataReader&&) = delete;
            TreeDataReader& operator=(TreeDataReader&&) = delete;
            ~TreeDataReader() = default;

            /*
            * 读取数据
            * 返回读取的块的索引
            */
            // Binary read(size_t index, size_t size){
            //     Binary binary_data;
            //     int visit_index = index / (BwtFS::BLOCK_SIZE - sizeof(uint8_t));
            //     if (visit_index >= m_visit_nodes->size()){
            //         LOG_ERROR << "Get Tree Data: Out of range";
            //         throw std::runtime_error("Get Tree Data: Out of range");
            //     }
            //     int node_data_start = index - visit_index * (BwtFS::BLOCK_SIZE - sizeof(uint8_t));
            //     size_t size_ = size;
            //     while(binary_data.size() < size){
            //         auto node = m_visit_nodes->at(visit_index);
            //         Binary data = m_fs->read(node.bitmap);
            //         white_node<RCAEncryptor> wnode(
            //             data, node.level, node.seed, node.start, node.length);
            //         int read_size = std::min(wnode.data().size() - node_data_start, size_);
            //         // LOG_INFO << "Read size: " << read_size;
            //         binary_data.append(wnode.data().read(node_data_start, read_size)); 
            //         node_data_start = 0;
            //         size_ -= read_size;
            //         // LOG_INFO << "Size: " << size_ << ", visit_index: " << visit_index;
            //         if (size_ <= 0){
            //             break;
            //         }
            //         visit_index++;
            //         if (visit_index >= m_visit_nodes->size()){
            //             break;
            //         }
            //     }
            //     return binary_data;
            // }
            Binary read(size_t index, size_t size){
                Binary binary_data;
                int visit_index = index / (BwtFS::BLOCK_SIZE - sizeof(uint8_t));
                if (visit_index >= m_visit_nodes->size()){
                    LOG_WARNING << "Get Tree Data: Out of range: " << visit_index 
                              << ", size: " << m_visit_nodes->size();
                    // throw std::runtime_error("Get Tree Data: Out of range");
                    return binary_data; // 返回空的Binary
                }
                int node_data_start = index - visit_index * (BwtFS::BLOCK_SIZE - sizeof(uint8_t));
                size_t size_ = size;
                while(binary_data.size() < size){
                    // LOG_DEBUG << "Total size: " << binary_data.size() 
                    //           << ", size: " << size 
                    //           << ", visit_index: " << visit_index;
                    auto node = m_visit_nodes->at(visit_index);
                    Binary data = m_fs->read(node.bitmap);
                    
                    white_node<RCAEncryptor> wnode(
                        data, node.level, node.seed, node.start, node.length);
                    int read_size;
                    if (wnode.data().size() - node_data_start < size_){
                        read_size = wnode.data().size() - node_data_start;
                        binary_data.append(wnode.data().read(node_data_start, read_size)); 
                        node_data_start = 0;
                    }else{
                        read_size = size_;
                        binary_data.append(wnode.data().read(node_data_start, read_size)); 
                        node_data_start = 0;
                        break;
                    }
                    // LOG_INFO << "Read size: " << read_size 
                    //           << ", node_data_start: " << node_data_start 
                    //           << ", size_: " << size_
                    //           << ", binary_data size: " << binary_data.size();
                    size_ -= read_size;
                    if (size_ <= 0){
                        break;
                    }
                    visit_index++;
                    if (visit_index >= m_visit_nodes->size()){
                        break;
                    }
                }
                return binary_data;
            }

        int get_count(){
            return m_visit_nodes->size();
        }

        void delete_file(){
            // 删除访问节点
            for (auto& bitmap : delete_bitmap){
                m_fs->bitmap->clear(bitmap);
            }
            for (auto it = m_visit_nodes->begin(); it != m_visit_nodes->end(); ++it){
                m_fs->bitmap->clear(it->bitmap);
            }
        }

        private:
            std::shared_ptr<BwtFS::System::FileSystem> m_fs;
            std::queue<entry> m_entry_queue;
            secure_ptr<std::vector<VisitNode>> m_visit_nodes = 
                    make_secure<std::vector<VisitNode>>();
            std::vector<size_t> delete_bitmap;
            /*
            * 初始化访问节点
            */
            void init(bool is_delete = false){
                LOG_DEBUG << "Init visit nodes";
                while(!m_entry_queue.empty()){
                    auto entry = m_entry_queue.front();
                    m_entry_queue.pop();
                    LOG_DEBUG << "Entry bitmap: " << entry.get_bitmap() 
                              << ", level: " << (int)entry.get_level() 
                              << ", seed: " << entry.get_seed() 
                              << ", start: " << entry.get_start() 
                              << ", length: " << entry.get_length();
                    Binary bd = m_fs->read(entry.get_bitmap());
                    LOG_DEBUG << "Read bitmap: " << entry.get_bitmap() 
                              << ", size: " << bd.size();
                    if(is_delete){
                        delete_bitmap.push_back(entry.get_bitmap());
                    }
                    black_node<RCAEncryptor> node(
                        bd, entry.get_level(), entry.get_seed(),
                         entry.get_start(), entry.get_length());
                    std::vector<BwtFS::Node::entry> entries;
                    for (int i = 0; i < node.get_size_of_entry(); i++){
                        auto e = node.get_entry(i);
                        entries.push_back(e);
                    }
                    for (int i = 0; i < entries.size(); i++){
                        auto e = entries[i];
                        VisitNode node;
                        node.bitmap = e.get_bitmap();
                        if (node.bitmap <= 0){
                            LOG_ERROR << "Bitmap is 0, entry: " << e.get_bitmap();
                            throw std::runtime_error("Bitmap is 0");
                        }
                        node.start = e.get_start();
                        node.length = e.get_length();
                        node.seed = e.get_seed();
                        node.level = e.get_level();
                        if (e.get_type() == NodeType::BLACK_NODE){
                            m_entry_queue.emplace(node.bitmap, NodeType::BLACK_NODE, 
                                                    node.start, node.length, 
                                                    node.seed, node.level);
                        }else{
                            m_visit_nodes->push_back(node);
                        }
                    }
                }
            }
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

            bw_tree(const std::string & token, bool is_delete = false){
                try {
                    m_tree_data_reader = new TreeDataReader(token, is_delete);
                } catch (const std::exception& e) {
                    LOG_ERROR << "Failed to initialize bw_tree, error: " << e.what();
                    throw std::runtime_error("Failed to initialize bw_tree");
                }
            }

            bw_tree(const bw_tree&) = delete;
            bw_tree& operator=(const bw_tree&) = delete;
            bw_tree(bw_tree&&) = delete;
            bw_tree& operator=(bw_tree&&) = delete;
            ~bw_tree(){
                if (m_tree_data_reader != nullptr){
                    delete m_tree_data_reader;
                    m_tree_data_reader = nullptr;
                }
            }
            /*
            * 将数据写入黑白树
            */
            void write(char* data, size_t size){
                if (this->m_cache_data == nullptr){
                    this->get_node();
                }
                size_t used_size = 0;
                while(size){
                    size_t copy_size = std::min(size, SIZE_OF_NODE_DATA - m_cache_data->size);
                    std::memcpy(m_cache_data->data + m_cache_data->size, data + used_size, copy_size);
                    size -= copy_size;
                    m_cache_data->size += copy_size;
                    used_size += copy_size;
                    // LOG_INFO << "copy_size: " << copy_size << ", size: " << size << ", m_cache_data->size: " << m_cache_data->size;
                    if (m_cache_data->size == SIZE_OF_NODE_DATA){
                        // LOG_INFO << std::string(reinterpret_cast<char*>(m_cache_data->data), SIZE_OF_NODE_DATA);
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
                    // LOG_INFO << std::string(reinterpret_cast<char*>(m_cache_data->data), SIZE_OF_NODE_DATA);
                    m_nodes.enqueue(m_cache_data);
                }
                set_write_finished(true);
            }
            /*
            * 从黑白树中读取数据
            */
            Binary read(size_t index, size_t size){
                // 读取数据的逻辑
                return m_tree_data_reader->read(index, size);
            }

            int get_visit_node_count(){
                return m_tree_data_reader->get_count();
            }

            int get_node_count(){
                return m_nodes.size();
            }

            void delete_file(){
                m_tree_data_reader->delete_file();
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
                std::queue<black_node<RCAEncryptor>*> bkn_queue;
                black_node<RCAEncryptor>* bkn = new black_node<RCAEncryptor>(0);
                constexpr int entry_num = BwtFS::BLOCK_SIZE / SIZE_OF_ENTRY;
                constexpr int max_level = 2;
                auto seeds = BwtFS::Util::RandNumbers<uint16_t>(entry_num, std::hash<std::queue<black_node<RCAEncryptor>*>*>{}(&bkn_queue), 1, 1 << 15);
                auto levels = BwtFS::Util::RandNumbers<uint8_t>(entry_num, std::hash<std::vector<uint16_t>*>{}(&seeds), 1, 1 << max_level);
                uint16_t seed;
                uint8_t level;
                while(!is_write_finished() || !m_nodes.empty()){
                    while(!m_nodes.empty()){
                        auto index = bkn->size();
                        seed = seeds.back();
                        level = levels.back();
                        seeds.pop_back();
                        levels.pop_back();
                        if (seeds.empty()){
                            seeds = BwtFS::Util::RandNumbers<uint16_t>(entry_num, std::hash<std::queue<black_node<RCAEncryptor>*>*>{}(&bkn_queue), 1, 1 << 15);
                            levels = BwtFS::Util::RandNumbers<uint8_t>(entry_num, std::hash<std::vector<uint16_t>*>{}(&seeds), 1, 1 << max_level);
                        }
                        auto node = get_node(index, seed, level);
                        auto bitmap = m_transaction_writer.write(node.data);
                        auto entry = generate_entry(bitmap, node.start, node.length, seed, level, false);
                        bkn->add_entry(entry);
                        if (bkn->is_fill()){
                            bkn_queue.push(bkn);
                            bkn = new black_node<RCAEncryptor>(0);
                        }
                    }
                }
                if (bkn->size() > 0){
                    bkn_queue.push(bkn);
                }
                // LOG_INFO << "White Data Write Finished";
                bkn = new black_node<RCAEncryptor>(0);
                std::queue<black_node<RCAEncryptor>*> bkn_queue_temp;
                bool is_temp = false;
                while(!bkn_queue.empty() || !bkn_queue_temp.empty()){
                    // LOG_INFO << "Queue size: " << bkn_queue.size() << ", temp queue size: " << bkn_queue_temp.size();
                    if (is_temp){
                        auto bkn_tmp = bkn_queue_temp.front();
                        bkn_queue_temp.pop();
                        seed = seeds.back();
                        level = levels.back();
                        seeds.pop_back();
                        levels.pop_back();
                        if (seeds.empty()){
                            seeds = BwtFS::Util::RandNumbers<uint16_t>(entry_num, std::hash<std::queue<black_node<RCAEncryptor>*>*>{}(&bkn_queue), 1, 1 << 15);
                            levels = BwtFS::Util::RandNumbers<uint8_t>(entry_num, std::hash<std::vector<uint16_t>*>{}(&seeds), 1, 1 << max_level);
                        }
                        bkn_tmp->set_index(bkn->size());
                        auto binary_data = bkn_tmp->to_binary(seed, level);
                        auto bitmap = m_transaction_writer.write(binary_data);
                        auto entry = generate_entry(bitmap, bkn_tmp->get_start(), bkn_tmp->get_length(), seed, level, true);
                        delete bkn_tmp;
                        bkn->add_entry(entry);
                        if (bkn->is_fill()){
                            bkn_queue_temp.push(bkn);
                            bkn = new black_node<RCAEncryptor>(0);
                        }
                        if(bkn_queue_temp.empty()){
                            is_temp = false;
                            if (bkn_queue.size() == 0){
                                bkn_queue.push(bkn);
                                break;
                            }
                            if (bkn->size() > 0){
                                bkn_queue_temp.push(bkn);
                            }

                        }
                    }else{
                        auto bkn_tmp = bkn_queue.front();
                        bkn_queue.pop();
                        seed = seeds.back();
                        level = levels.back();
                        seeds.pop_back();
                        levels.pop_back();
                        if (seeds.empty()){
                            seeds = BwtFS::Util::RandNumbers<uint16_t>(entry_num, std::hash<std::queue<black_node<RCAEncryptor>*>*>{}(&bkn_queue), 1, 1 << 15);
                            levels = BwtFS::Util::RandNumbers<uint8_t>(entry_num, std::hash<std::vector<uint16_t>*>{}(&seeds), 1, 1 << max_level);
                        }
                        bkn_tmp->set_index(bkn->size());
                        auto binary_data = bkn_tmp->to_binary(seed, level);
                        auto bitmap = m_transaction_writer.write(binary_data);
                        auto entry = generate_entry(bitmap, bkn_tmp->get_start(), bkn_tmp->get_length(), seed, level, true);
                        delete bkn_tmp;
                        bkn->add_entry(entry);
                        if (bkn->is_fill()){
                            bkn_queue_temp.push(bkn);
                            bkn = new black_node<RCAEncryptor>(0);
                        }
                        if(bkn_queue.empty()){
                            is_temp = true;
                            if (bkn_queue_temp.size() == 0){
                                bkn_queue_temp.push(bkn);
                                break;
                            }
                            if (bkn->size() > 0){
                                bkn_queue_temp.push(bkn);
                            }
                            
                        }

                    }
                }
                if(bkn_queue.size() == 1){
                    bkn = bkn_queue.front();
                    bkn_queue.pop();
                }else{
                    bkn = bkn_queue_temp.front();
                    bkn_queue_temp.pop();
                }
                auto binary_data = bkn->to_binary(seed, level);
                auto bitmap = m_transaction_writer.write(binary_data);
                // LOG_INFO << "Bitmap of token: " << bitmap;
                
                this->m_transaction_writer.set_write_finished(true);
                m_token = generate_token(bitmap, bkn->get_start(), bkn->get_length(), seed, level);
                is_generate = true;
                // LOG_INFO << "Token generated: " << m_token;
                // LOG_INFO << "bitmap: " << bitmap 
                //          << ", start: " << bkn->get_start() 
                //          << ", length: " << bkn->get_length() 
                //          << ", seed: " << seed 
                //          << ", level: " << (int)level;
                delete bkn;
                return m_token;
            }

            bool is_generate_tree(){
                return is_generate;
            }

            void join(){
                while(!is_generate){
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
            TreeDataReader* m_tree_data_reader = nullptr;
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