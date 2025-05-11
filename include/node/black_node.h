// #ifndef BLACK_NODE_H
// #define BLACK_NODE_H
// #include "node/base_node.h"
// #include "node/binary.h"
// #include "node/entry.h"
// namespace BwtFS::Node{
//     /*
//     * BlackNode类
//     * 该类是一个节点的实现类，继承自BaseNode
//     * 黑节点：存储索引
//     * @author: zaoweiceng
//     * @date: 2025-04-02
//     */
//     class BlackNode : public BaseNode {
//         public:
//             BlackNode(node_size start, node_size length, BwtFS::Node::Binary data, BwtFS::Node::NodeType type);
//             ~BlackNode() override = default;
//             // 读取数据
//             // 黑节点不存储数据，返回空
//             BwtFS::Node::Binary read(const unsigned index, node_size length) override;
//             // 写入数据
//             // 黑节点不存储数据，不执行操作
//             void write(const unsigned index, BwtFS::Node::Binary& data) override;
//             // 写入索引
//             // 在索引的指定位置添加一个索引，若索引已存在，则覆盖
//             void writeEntry(const unsigned index, const BwtFS::Node::Entry& entry);
//             // 写入索引
//             // 在索引的末尾添加一个索引
//             void writeEntry(const BwtFS::Node::Entry& entry);
//             // 写入索引
//             // 在索引的末尾添加多个索引
//             void writeEntry(const std::vector<BwtFS::Node::Entry>& entries);
//             // 读取索引
//             // 读取索引的指定位置的索引
//             BwtFS::Node::Entry readEntry(const unsigned index);
//             // 读取全部索引
//             // 读取索引的全部索引
//             std::vector<BwtFS::Node::Entry> readEntries() const;
//             // 反序列化
//             // 传入数据BLOCK_SIZE大小的Binary，返回反序列化的数据
//             // 黑节点的反序列化数据为索引的反序列化数据
//             void fromBinary(const BwtFS::Node::Binary& data) override;
//             // 序列化
//             // 返回序列化的数据
//             // 返回BLOCK_SIZE大小的Binary
//             BwtFS::Node::Binary toBinary() override;
//             // 获取有效数据的长度
//             // 黑节点不存储数据，返回0
//             size_t getLength() const;
//             // 获取空余数据的长度
//             // 黑节点不存储数据，返回BLOCK_SIZE
//             size_t getFreeLength() const;
//             // 获取有效索引的长度
//             size_t getEntriesLength() const;
//             // 获取索引的空余长度
//             size_t getFreeEntriesLength() const;
//             // 获取节点的类型
//             BwtFS::Node::NodeType getType() override;

//         private:
//             BwtFS::Node::Binary data;                   // 节点内数据
//             BwtFS::Node::EntryNode entries;             // 节点内索引
//             BwtFS::Node::NodeType type;                 // 节点类型
//             node_size start;                            // 节点内数据起始位置
//             node_size length;                           // 节点内数据长度
//     };
// }


// #endif