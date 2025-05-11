#include "node/bw_tree.h"
#include "node/entry.h"
#include "util/log.h"
#include "gtest/gtest.h"

using BwtFS::Util::Logger;
using BwtFS::Node::entry;
using BwtFS::Node::NodeType;
using BwtFS::Node::entry_list;
using BwtFS::Node::Binary;
TEST(EntryTest, serializeEntry){
    entry_list list;
    for (int i = 0; i < 1; i++){
        entry e(std::time(nullptr), NodeType::BLACK_NODE, 1, 4096, std::time(nullptr), 1);
        list.add_entry(std::move(e));
    }
    Binary binary_data = list.to_binary();
    // LOG_INFO << "Entry list size: " << list.size();
    // LOG_INFO << "Entry list size: " << binary_data.size();
    // LOG_INFO << "Entry list: " << binary_data.to_hex_string();
    entry_list new_list = entry_list::from_binary(binary_data, 1);
    
    for (size_t i = 0; i < new_list.size(); i++){
        entry e = new_list.get_entry(i);
        EXPECT_EQ(e.get_type(), NodeType::BLACK_NODE);
        EXPECT_EQ(e.get_start(), 1);
        EXPECT_EQ(e.get_length(), 4096);
        EXPECT_EQ(e.get_level(), 1);
    }
}