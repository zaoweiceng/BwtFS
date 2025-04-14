#include "node/entry.h"
#include "util/log.h"
#include "gtest/gtest.h"

using BwtFS::Util::Logger;

TEST(EntryTest, serializeEntry){
    BwtFS::Node::Entry entry;
    entry.setIndex(99564);
    entry.setRCA(true);
    entry.setLevel(2);
    entry.setType(BwtFS::Node::NodeType::WHITE_NODE);
    entry.setState(BwtFS::Node::NodeState::IN_USED);
    entry.setSeed(65530);

    auto data = entry.toBinary();
    // LOG_INFO << data.to_hex_string();
    auto new_entry = BwtFS::Node::Entry::fromBinary(data);

    EXPECT_EQ(entry.getIndex(), new_entry->getIndex());
    EXPECT_EQ(entry.isRCA(), new_entry->isRCA());
    EXPECT_EQ(entry.getLevel(), new_entry->getLevel());
    EXPECT_EQ(entry.getType(), new_entry->getType());
    EXPECT_EQ(entry.getState(), new_entry->getState());
    EXPECT_EQ(entry.getSeed(), new_entry->getSeed());
}

TEST(EntryTest, EntryNode){
    BwtFS::Node::Entry entry = BwtFS::Node::Entry()
                                .setIndex(1)
                                .setRCA(true)
                                .setLevel(2)
                                .setType(BwtFS::Node::NodeType::WHITE_NODE)
                                .setState(BwtFS::Node::NodeState::IN_USED)
                                .setSeed(65530);
    BwtFS::Node::Entry entry1 = BwtFS::Node::Entry()
                                .setIndex(2)
                                .setRCA(false)
                                .setLevel(3)
                                .setType(BwtFS::Node::NodeType::WHITE_NODE)
                                .setState(BwtFS::Node::NodeState::IN_USED)
                                .setSeed(65531);
    
    BwtFS::Node::EntryNode entry_node;
    entry_node.addEntry(entry).addEntry(std::move(entry1));
    EXPECT_EQ(entry_node.getEntryCount(), 2);
    EXPECT_EQ(entry_node.getEntry(0)->getIndex(), 1);
    auto data = entry_node.toBinary();
    BwtFS::Node::EntryNode entry_node1 = BwtFS::Node::EntryNode::fromBinary(data, 2);
    EXPECT_EQ(entry_node1.getEntryCount(), 2);
    EXPECT_EQ(entry_node1.getEntry(0)->getIndex(), 1);
    EXPECT_EQ(entry_node1.getEntry(1)->getIndex(), 2);
    EXPECT_EQ(entry_node1.getEntry(0)->isRCA(), true);
    EXPECT_EQ(entry_node1.getEntry(1)->isRCA(), false);
}