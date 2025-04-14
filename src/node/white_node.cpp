#include "node/white_node.h"
#include "util/log.h"
#include "util/random.h"

using BwtFS::Util::Logger;

BwtFS::Node::WhiteNode::WhiteNode(node_size start, node_size length, BwtFS::Node::Binary data, BwtFS::Node::NodeType type)
 : BaseNode(start, length, type), data(data) {
    // 构造函数
}

BwtFS::Node::WhiteNode::WhiteNode(const BwtFS::Node::Binary& data)
 : BaseNode(0, 0, BwtFS::Node::NodeType::WHITE_NODE) {
    // 构造函数
    if (data.size() != BLOCK_SIZE) {
        throw std::runtime_error("Data size is not equal to BLOCK_SIZE. "
             + std::string(__FILE__) + ":" + std::to_string(__LINE__));
    }
    this->fromBinary(data);
}

BwtFS::Node::WhiteNode::~WhiteNode() {
    // 析构函数
    this->data.clear();
}

BwtFS::Node::Binary BwtFS::Node::WhiteNode::read(const unsigned index, node_size length){
    node_size length_ = length;
    if (length_ > this->length) {
        length_ = this->length;
    }
    if (length_ <= 0) {
        LOG_WARNING << "read length is less than 0, return empty data";
        return BwtFS::Node::Binary();
    }
    if (index < 0){
        LOG_WARNING << "read index is less than 0, return empty data";
        return BwtFS::Node::Binary(0);
    }
    return this->data.read(index, length_);
}

BwtFS::Node::Binary BwtFS::Node::WhiteNode::read() const {
    return this->data.read(0, this->length);
}

void BwtFS::Node::WhiteNode::write(const unsigned index, BwtFS::Node::Binary& data){
    if (index < 0){
        LOG_WARNING << "write index is less than 0, return empty data";
        return;
    }
    if (index + data.size() >= this->data.size()){
        LOG_WARNING << "write index is out of range, return empty data";
        return;
    }
    this->data.write(index, data.read(0, data.size()));
}

void BwtFS::Node::WhiteNode::write(const BwtFS::Node::Binary& data){
    this->data.write(0, data.read(0, data.size()));
}

BwtFS::Node::WhiteNode& BwtFS::Node::WhiteNode::initWhiteNode(BwtFS::Node::Binary data) {
    auto start = BwtFS::Util::RandNumber(std::time(nullptr), 2*sizeof(BwtFS::node_size) + sizeof(uint8_t), BLOCK_SIZE - 1 - data.size());
    auto length = data.size();
    return *(new BwtFS::Node::WhiteNode(start, length, data, BwtFS::Node::NodeType::WHITE_NODE));
}

size_t BwtFS::Node::WhiteNode::getLength() const {
    return this->length;
}

size_t BwtFS::Node::WhiteNode::getFreeLength() const {
    return BLOCK_SIZE - this->length;
}

BwtFS::Node::NodeType BwtFS::Node::WhiteNode::getType() {
    return this->type;
}

BwtFS::Node::Binary BwtFS::Node::WhiteNode::toBinary() {
    auto randomData = BwtFS::Util::RandBytes(BLOCK_SIZE, std::time(nullptr), 0, 255);
    BwtFS::Node::Binary data(randomData);
    data.write(0, sizeof(node_size), reinterpret_cast<std::byte*>(&this->start));
    data.write(sizeof(node_size), sizeof(node_size), reinterpret_cast<std::byte*>(&this->length));
    data.write(2*sizeof(node_size), sizeof(uint8_t), reinterpret_cast<std::byte*>(&this->type));
    data.write(this->start, this->data.read(0, this->length));
    return data;
}

void BwtFS::Node::WhiteNode::fromBinary(const BwtFS::Node::Binary& data) {
    if (data.size() != BLOCK_SIZE) {
        LOG_ERROR << "Data size is not equal to BLOCK_SIZE.";
        throw std::runtime_error("Data size is not equal to BLOCK_SIZE. "
             + std::string(__FILE__) + ":" + std::to_string(__LINE__));
    }
    this->start = *(node_size*)data.read(0, sizeof(node_size)).data();
    this->length = *(node_size*)data.read(sizeof(node_size), sizeof(node_size)).data();
    this->type = (BwtFS::Node::NodeType)*(uint8_t*)data.read(2*sizeof(node_size), sizeof(uint8_t)).data();
    this->data = BwtFS::Node::Binary(data.read(this->start, this->length));
}