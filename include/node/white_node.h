// #ifndef WHITE_NODE_H
// #define WHITE_NODE_H
// #include "node/base_node.h"
// #include "node/binary.h"
// namespace BwtFS::Node{
//     /*
//     * WhiteNode类
//     * 该类是一个节点的实现类，继承自BaseNode
//     * 白节点：存储数据
//     * @author: zaoweiceng
//     * @date: 2025-04-02
//     */
//     class WhiteNode : public BaseNode {
//         public:
//             // 构造函数
//             WhiteNode(node_size start, node_size length, BwtFS::Node::Binary data, BwtFS::Node::NodeType type);
//             WhiteNode(const BwtFS::Node::Binary& data);
//             // 析构函数
//             ~WhiteNode() override;
//             // 读取数据
//             BwtFS::Node::Binary read(const unsigned index, node_size length) override;
//             // 读取全部数据
//             BwtFS::Node::Binary read() const;
//             // 写入数据
//             void write(const unsigned index, BwtFS::Node::Binary& data) override;
//             // 写入全部数据
//             void write(const BwtFS::Node::Binary& data);
//             // 反序列化
//             void fromBinary(const BwtFS::Node::Binary& data) override;
//             // 序列化
//             BwtFS::Node::Binary toBinary() override;
//             // 获取有效数据的长度
//             size_t getLength() const;
//             // 获取空余数据的长度
//             size_t getFreeLength() const;
//             // 获取节点的类型
//             BwtFS::Node::NodeType getType() override;
//             // 初始化节点
//             static WhiteNode& initWhiteNode(BwtFS::Node::Binary data);
            

//         private:
//             BwtFS::Node::Binary data;       // 节点内数据
//     };
// }

// #endif