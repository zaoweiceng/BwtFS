#include "node/entry.h"
#include "util/log.h"

using BwtFS::Util::Logger;

size_t BwtFS::Node::Entry::getIndex() const {
    return this->index;
}
BwtFS::Node::Entry& BwtFS::Node::Entry::setIndex(size_t index) {
    this->index = index;
    return *this;
}
bool BwtFS::Node::Entry::isRCA() const {
    return this->is_rca;
}
BwtFS::Node::Entry& BwtFS::Node::Entry::setRCA(bool is_rca) {
    this->is_rca = is_rca;
    return *this;
}
uint8_t BwtFS::Node::Entry::getLevel() const {
    return this->level;
}
BwtFS::Node::Entry& BwtFS::Node::Entry::setLevel(uint8_t level) {
    this->level = level;
    return *this;
}
BwtFS::Node::NodeType BwtFS::Node::Entry::getType() const {
    return this->type;
}
BwtFS::Node::Entry& BwtFS::Node::Entry::setType(NodeType type) {
    this->type = type;
    return *this;
}
BwtFS::Node::NodeState BwtFS::Node::Entry::getState() const {
    return this->state;
}
BwtFS::Node::Entry& BwtFS::Node::Entry::setState(NodeState state) {
    this->state = state;
    return *this;
}
unsigned BwtFS::Node::Entry::getSeed() const {
    return this->seed;
}
BwtFS::Node::Entry& BwtFS::Node::Entry::setSeed(unsigned seed) {
    this->seed = seed;
    return *this;
}
std::shared_ptr<BwtFS::Node::Entry> BwtFS::Node::Entry::fromBinary(const BwtFS::Node::Binary& data) {
    auto entry = std::make_shared<BwtFS::Node::Entry>();
    entry->index = reinterpret_cast<size_t&>(data.read(0, sizeof(size_t))[0]);
    entry->is_rca = reinterpret_cast<bool&>(data.read(sizeof(size_t), sizeof(bool))[0]);
    entry->level = reinterpret_cast<uint8_t&>(data.read(sizeof(size_t) + sizeof(bool), sizeof(uint8_t))[0]);
    entry->type = static_cast<BwtFS::Node::NodeType>(reinterpret_cast<uint8_t&>(data.read(sizeof(size_t) + sizeof(bool) + sizeof(uint8_t), sizeof(uint8_t))[0]));
    entry->state = static_cast<BwtFS::Node::NodeState>(reinterpret_cast<uint8_t&>(data.read(sizeof(size_t) + sizeof(bool) + sizeof(uint8_t) * 2, sizeof(uint8_t))[0]));
    entry->seed = reinterpret_cast<unsigned&>(data.read(sizeof(size_t) + sizeof(bool) + sizeof(uint8_t) * 3, sizeof(unsigned))[0]);
    return entry;
}
BwtFS::Node::Binary BwtFS::Node::Entry::toBinary() {
    BwtFS::Node::Binary data(Entry::ENTRY_SIZE);
    data.write(0, sizeof(size_t), reinterpret_cast<std::byte*>(&this->index));
    data.write(sizeof(size_t), sizeof(bool),  reinterpret_cast<std::byte*>(&this->is_rca));
    data.write(sizeof(size_t) + sizeof(bool), sizeof(uint8_t), reinterpret_cast<std::byte*>(&this->level));
    data.write(sizeof(size_t) + sizeof(bool) + sizeof(uint8_t), sizeof(uint8_t),  reinterpret_cast<std::byte*>(&this->type));
    data.write(sizeof(size_t) + sizeof(bool) + sizeof(uint8_t) * 2, sizeof(uint8_t), reinterpret_cast<std::byte*>(&this->state));
    data.write(sizeof(size_t) + sizeof(bool) + sizeof(uint8_t) * 3, sizeof(unsigned), reinterpret_cast<std::byte*>(&this->seed));
    return data;
}

BwtFS::Node::EntryNode BwtFS::Node::EntryNode::fromBinary(const BwtFS::Node::Binary& data, size_t size) {
    auto entry_node = BwtFS::Node::EntryNode();
    for (size_t i = 0; i < size; i++){
        auto entry_data = data.read(i * BwtFS::Node::Entry::ENTRY_SIZE, BwtFS::Node::Entry::ENTRY_SIZE);
        auto entry = BwtFS::Node::Entry::fromBinary(entry_data);
        entry_node.addEntry(std::move(*entry));
    }
    return entry_node;
}

BwtFS::Node::Binary BwtFS::Node::EntryNode::toBinary() {
    BwtFS::Node::Binary data(0);
    for (size_t i = 0; i < this->entry_count; i++){
        data += this->entries->at(i).toBinary();
    }
    return data;
}

size_t BwtFS::Node::EntryNode::getEntryCount() const {
    return this->entry_count;
}
BwtFS::Node::EntryNode& BwtFS::Node::EntryNode::setEntryCount(size_t count) {
    this->entry_count = count;
    if (this->entries == nullptr) {
        this->entries = std::make_shared<std::vector<BwtFS::Node::Entry>>(count);
        return *this;
    }
    if (this->entries->size() < count) {
        this->entries->resize(count);
    }
    else if (this->entries->size() > count) {
        this->entries->resize(count);
    }
    else {
        return *this;
    }
    this->entries = std::make_shared<std::vector<BwtFS::Node::Entry>>(count);
    return *this;
}

std::shared_ptr<BwtFS::Node::Entry> BwtFS::Node::EntryNode::getEntry(size_t index) const {
    return std::make_shared<BwtFS::Node::Entry>(this->entries->at(index));
}

BwtFS::Node::EntryNode& BwtFS::Node::EntryNode::setEntry(size_t index, BwtFS::Node::Entry& entry) {
    if (index >= this->entry_count) {
        LOG_ERROR << "Index out of range: " << index << ", entry_count: " << this->entry_count;
        throw std::out_of_range("Index out of range" + std::to_string(index) 
        + ", entry_count: " + std::to_string(this->entry_count) + __FILE__ + ":" + std::to_string(__LINE__));
    }
    if (index < 0) {
        throw std::out_of_range("Index out of range" + std::to_string(index)
        + ", entry_count: " + std::to_string(this->entry_count) + __FILE__ + ":" + std::to_string(__LINE__));
    }
    this->entries->at(index) = entry;
    return *this;
}

BwtFS::Node::EntryNode& BwtFS::Node::EntryNode::setEntry(size_t index, BwtFS::Node::Entry&& entry) {
    if (index >= this->entry_count) {
        LOG_ERROR << "Index out of range: " << index << ", entry_count: " << this->entry_count;
        throw std::out_of_range("Index out of range" + std::to_string(index) 
        + ", entry_count: " + std::to_string(this->entry_count) + __FILE__ + ":" + std::to_string(__LINE__));
    }
    if (index < 0) {
        throw std::out_of_range("Index out of range" + std::to_string(index)
        + ", entry_count: " + std::to_string(this->entry_count) + __FILE__ + ":" + std::to_string(__LINE__));
    }
    this->entries->at(index) = std::move(entry);
    return *this;
}

BwtFS::Node::EntryNode& BwtFS::Node::EntryNode::addEntry(BwtFS::Node::Entry& entry) {
    this->entries->push_back(entry);
    this->entry_count++;
    return *this;
}

BwtFS::Node::EntryNode& BwtFS::Node::EntryNode::addEntry(BwtFS::Node::Entry&& entry) {
    this->entries->push_back(std::move(entry));
    this->entry_count++;
    return *this;
}

BwtFS::Node::EntryNode::EntryNode() {
    this->entry_count = 0;
    this->entries = std::make_shared<std::vector<BwtFS::Node::Entry>>();
}
