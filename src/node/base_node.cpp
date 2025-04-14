#include "node/base_node.h"

BwtFS::Node::BaseNode::BaseNode(node_size start, node_size length, BwtFS::Node::NodeType type)
    : start(start), length(length), type(type) {
}