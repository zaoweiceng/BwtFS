#ifndef ENTRY_H
#define ENTRY_H
#include "node/binary.h"
#include "node/base_node.h"
#include "config.h"
namespace BwtFS::Node{
    /*
    * NodeState类
    * 标记节点的状态
    * IN_USED: 节点在使用中
    * FREE: 节点未使用
    * DELETED: 节点已删除
    * 该状态用于标记节点的使用情况
    */
    enum class NodeState{
        IN_USED = 0, // 节点在使用中
        FREE = 1,   // 节点未使用
        DELETED = 2 // 节点已删除
    };
    /*
    * Entry类
    * 该类是一个节点的实现类，继承自BaseNode
    * 该类为节点的索引
    * @author: zaoweiceng
    * @date: 2025-04-02
    */
    class Entry {
        public:
            Entry() = default;
            ~Entry() = default;
           
            // 反序列化
            // 传入数据，返回序列化的数据
            static std::shared_ptr<BwtFS::Node::Entry> fromBinary(const BwtFS::Node::Binary& data);
            // 序列化
            // 返回序列化的数据
            BwtFS::Node::Binary toBinary();

            size_t getIndex() const;            // 获取节点索引
            Entry& setIndex(size_t index);        // 设置节点索引
            bool isRCA() const;                 // 获取是否启用RCA
            Entry& setRCA(bool is_rca);           // 设置是否启用RCA
            uint8_t getLevel() const;           // 获取RCA级别
            Entry& setLevel(uint8_t level);       // 设置RCA级别
            NodeType getType() const;           // 获取节点类型
            Entry& setType(NodeType type);        // 设置节点类型
            NodeState getState() const;         // 获取节点状态
            Entry& setState(NodeState state);     // 设置节点状态
            unsigned getSeed() const;           // 获取随机数种子
            Entry& setSeed(unsigned seed);        // 设置随机数种子

            const static size_t ENTRY_SIZE = 16; // 节点索引的大小
        private:
            size_t index;       // 节点索引         8字节
            bool is_rca;        // 是否启用RCA      1字节
            uint8_t level;      // RCA级别          1字节
            NodeType type;      // 节点类型         1字节
            NodeState state;    // 节点状态         1字节
            unsigned seed;      // 随机数种子       4字节
    };
    class EntryNode{

        public:
            EntryNode();
            ~EntryNode() = default;
            // 反序列化
            // 传入数据，返回序列化的数据
            static BwtFS::Node::EntryNode fromBinary(const BwtFS::Node::Binary& data, size_t size);
            // 序列化
            // 返回序列化的数据
            BwtFS::Node::Binary toBinary();
            // 获取节点索引
            // 返回节点索引的数量
            size_t getEntryCount() const;
            // 设置节点索引
            // 传入节点索引的数量，设置节点索引的数量
            EntryNode& setEntryCount(size_t count);
            // 获取节点索引
            std::shared_ptr<BwtFS::Node::Entry> getEntry(size_t index) const;
            // 设置节点索引
            // 传入节点索引，设置节点索引
            EntryNode& setEntry(size_t index, BwtFS::Node::Entry& entry);
            EntryNode& setEntry(size_t index, BwtFS::Node::Entry&& entry);
            // 添加节点
            // 传入节点索引，添加节点索引
            EntryNode& addEntry(BwtFS::Node::Entry& entry);
            EntryNode& addEntry(BwtFS::Node::Entry&& entry);

        private:
            std::shared_ptr<std::vector<Entry>> entries; // 节点索引
            size_t entry_size;          // 节点索引的大小
            size_t entry_count;         // 节点索引的数量
    };
}

#endif