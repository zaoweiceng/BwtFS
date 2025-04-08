#ifndef GRAY_NODE_H
#define GRAY_NODE_H
#include "node/base_node.h"
#include "node/binary.h"
#include "node/entry.h"
namespace BwtFS::Node{
    /*
    * GrayNode类
    * 该类是一个节点的实现类，继承自BaseNode
    * 灰节点：存储数据和索引
    * @author: zaoweiceng
    * @date: 2025-04-02
    */
    class GrayNode:public BaseNode{
        public:
            GrayNode() = default;
            ~GrayNode() override = default;
            // 读取数据
            BwtFS::Node::Binary read(const unsigned long long index) override;
            // 读取全部数据
            BwtFS::Node::Binary read();
            // 读取索引
            BwtFS::Node::Entry readEntry(const unsigned index);
            // 写入数据
            void write(const unsigned long long index, BwtFS::Node::Binary& data) override;
            // 写入全部数据
            void write(const BwtFS::Node::Binary& data);
            // 写入索引
            // 在索引的指定位置添加一个索引，若索引已存在，则覆盖
            void writeEntry(const unsigned index, const BwtFS::Node::Entry& entry);
            // 写入索引
            // 在索引的末尾添加一个索引
            void writeEntry(const BwtFS::Node::Entry& entry);
            // 写入索引
            // 在索引的末尾添加多个索引
            void writeEntry(const std::vector<BwtFS::Node::Entry>& entries);
            // 序列化
            // 传入数据BLOCK_SIZE大小的Binary，返回序列化的数据
            void fromBinary(const BwtFS::Node::Binary& data) override;
            // 反序列化
            // 返回反序列化的数据
            // 返回BLOCK_SIZE大小的Binary
            BwtFS::Node::Binary toBinary() const override;


            // 获取有效数据的长度
            size_t getLength() const;
            // 获取空余数据的长度
            size_t getFreeLength() const;
            // 获取有效索引的长度
            size_t getEntriesLength() const;
            // 获取空余索引的长度
            size_t getFreeEntriesLength() const;
            // 获取节点的类型
            BwtFS::Node::NodeType getType() override;


        private:
            size_t start;                               // 节点内数据起始位置
            node_size length;                           // 节点内数据长度
            BwtFS::Node::Binary data;                   // 节点内数据
            std::vector<BwtFS::Node::Entry> entries;    // 节点内索引
            BwtFS::Node::NodeType type;                 // 节点类型
    };
}

#endif