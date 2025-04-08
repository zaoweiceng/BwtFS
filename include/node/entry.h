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
    class Entry: public BaseNode {
        public:
            Entry() = default;
            ~Entry() = default;

            // 读取数据
            // 传入索引，返回读取的数据
            BwtFS::Node::Binary read(const unsigned long long index);
            // 写入数据
            // 传入索引和数据，返回写入的数据
            void write(const unsigned long long index, BwtFS::Node::Binary& data);
            // 序列化
            // 传入数据，返回序列化的数据
            void fromBinary(const BwtFS::Node::Binary& data);
            // 反序列化
            // 返回反序列化的数据
            BwtFS::Node::Binary toBinary() const;

            const static size_t ENTRY_SIZE = 16; // 节点索引的大小
        private:
            size_t index;       // 节点索引         8字节
            bool is_rca;        // 是否启用RCA      1字节
            uint8_t level;      // RCA级别          1字节
            NodeType type;      // 节点类型         1字节
            NodeState state;    // 节点状态         1字节
            unsigned seed;      // 随机数种子       4字节
    };
}

#endif