# ifndef BASE_NODE_H
# define BASE_NODE_H
#include <cstddef>
#include <string>
#include <vector>
#include "node/binary.h"
#include "config.h"
namespace BwtFS::Node {
    /*
    * NodeType类
    * 标记节点的类型
    * WHITE_NODE: 白节点
    * BLACK_NODE: 黑节点
    * GRAY_NODE: 灰节点
    */
   enum class NodeType{
        WHITE_NODE = 0, // 白节点
        BLACK_NODE = 1, // 黑节点
        GRAY_NODE = 2   // 灰节点
    };
    /*
    * BaseNode类是一个抽象基类，定义了节点的基本操作接口
    * 该类包含了读取、写入、序列化和反序列化等操作
    * 数据存储结构示意图：
    *   ±--------------±-----------------±-------------±
    *   | 数据起始位置 |     数据长度     | 数据内容   | 
    *   ±--------------±-----------------±-------------±
    * 
    * 注：一个节点的大小若不足BLOCK_SIZE，则剩余部分随机填充，且start和length用于指定有效数据的位置
    */
    class BaseNode {
        public:
            BaseNode() = delete;
            BaseNode(node_size start, node_size length, BwtFS::Node::NodeType type);
            virtual ~BaseNode() = default;
            // 读取数据
            virtual BwtFS::Node::Binary read(const unsigned index, node_size length) = 0;
            // 写入数据
            virtual void write(const unsigned index, BwtFS::Node::Binary& data) = 0;
            // 反序列化
            virtual void fromBinary(const BwtFS::Node::Binary& data) = 0;
            // 序列化
            virtual BwtFS::Node::Binary toBinary() = 0;
            // 获取节点的类型
            virtual BwtFS::Node::NodeType getType() = 0;
        
        protected:
            node_size start;                // 节点内数据起始位置
            node_size length;               // 节点内数据长度
            BwtFS::Node::NodeType type;     // 节点类型
    };
    const static node_size NODE_FREE_SIZE = BLOCK_SIZE - sizeof(node_size) - sizeof(node_size); // 节点内数据的空余大小
}
#endif