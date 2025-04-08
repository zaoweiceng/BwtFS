#ifndef WHITE_NODE_H
#define WHITE_NODE_H
#include "node/base_node.h"
#include "node/binary.h"
namespace BwtFS::Node{
    /*
    * WhiteNode类
    * 该类是一个节点的实现类，继承自BaseNode
    * 白节点：存储数据
    * @author: zaoweiceng
    * @date: 2025-04-02
    */
    class WhiteNode : public BaseNode {
        public:
            WhiteNode() = default;
            ~WhiteNode() override = default;
            // 读取数据
            BwtFS::Node::Binary read(const unsigned long long index) override;
            // 读取全部数据
            BwtFS::Node::Binary read() const;
            // 写入数据
            void write(const unsigned long long index, BwtFS::Node::Binary& data) override;
            // 写入全部数据
            void write(const BwtFS::Node::Binary& data);
            // 序列化
            void fromBinary(const BwtFS::Node::Binary& data) override;
            // 反序列化
            BwtFS::Node::Binary toBinary() const override;
            // 获取有效数据的长度
            size_t getLength() const;
            // 获取空余数据的长度
            size_t getFreeLength() const;
            // 获取节点的类型
            BwtFS::Node::NodeType getType() override;

        private:
            size_t start;                   // 节点内数据起始位置
            node_size length;               // 节点内数据长度
            BwtFS::Node::Binary data;       // 节点内数据
            BwtFS::Node::NodeType type;     // 节点类型
    };
}

#endif