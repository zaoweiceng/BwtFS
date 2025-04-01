# ifndef BASE_NODE_H
# define BASE_NODE_H
#include <cstddef>
#include <string>
#include <vector>
#include "node/binary.h"
#include "config.h"
namespace BwtFS::Node {
    class BaseNode {
        public:
            virtual ~BaseNode() = default;
            virtual BwtFS::Node::Binary read(const unsigned long long index) = 0;
            virtual void write(const unsigned long long index, BwtFS::Node::Binary& data) = 0;
            virtual void fromBinary(const BwtFS::Node::Binary& data) = 0;
            virtual BwtFS::Node::Binary toBinary() const = 0;
        
        private:
            size_t start;
            node_size length;
            BwtFS::Node::Binary data;
    };
}
#endif