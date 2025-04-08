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
    */
    class BaseNode {
        public:
            virtual ~BaseNode() = default;
            // 读取数据
            virtual BwtFS::Node::Binary read(const unsigned long long index) = 0;
            // 写入数据
            virtual void write(const unsigned long long index, BwtFS::Node::Binary& data) = 0;
            // 反序列化
            virtual void fromBinary(const BwtFS::Node::Binary& data) = 0;
            // 序列化
            virtual BwtFS::Node::Binary toBinary() const = 0;
            // 获取节点的类型
            virtual BwtFS::Node::NodeType getType() = 0;
        
        private:
            size_t start;                   // 节点内数据起始位置
            node_size length;               // 节点内数据长度
            BwtFS::Node::Binary data;       // 节点内数据
            BwtFS::Node::NodeType type;     // 节点类型
    };
}
#endif