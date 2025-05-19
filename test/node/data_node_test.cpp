#include "node/entry.h"
#include "node/bw_node.h"
#include "util/log.h"
#include "gtest/gtest.h"
#include "util/secure_ptr.h"

using BwtFS::Util::Logger;

TEST(WhiteNode, baseTest){
    BwtFS::Node::Binary data("Hello world!!!", BwtFS::Node::StringType::ASCII);
    // LOG_INFO << "Binary: " << data.to_hex_string();
    auto wtn = BwtFS::Node::white_node<void>(data, 253);
    // LOG_INFO << "WhiteNode: " << wtn.to_binary().to_hex_string();
    auto binary_data = wtn.to_binary();
    auto wtn2 = BwtFS::Node::white_node<void>(binary_data, wtn.get_start(), wtn.get_length());
    EXPECT_EQ(wtn2.data().to_hex_string(), data.to_hex_string());
}

TEST(WhiteNode, encryptTest){
    BwtFS::Node::Binary data("Hello world!!!", BwtFS::Node::StringType::ASCII);
    // LOG_INFO << "Binary: " << data.to_hex_string();
    auto wtn = BwtFS::Node::white_node<RCAEncryptor>(data, 255);
    auto e_data = wtn.to_binary(255, 1);
    // LOG_INFO << "WhiteNode: " << e_data.to_hex_string();
    auto wtn2 = BwtFS::Node::white_node<RCAEncryptor>(e_data, 1, 255, wtn.get_start(), wtn.get_length());
    EXPECT_EQ(wtn2.get_index(), 255);
    EXPECT_EQ(wtn2.data().to_hex_string(), data.to_hex_string());
}

TEST(WhiteNode, mutiEncryptTest){
    BwtFS::Node::Binary data("Hello world!!!", BwtFS::Node::StringType::ASCII);
    // LOG_INFO << "Binary: " << data.to_hex_string();
    auto wtn = BwtFS::Node::white_node<RCAEncryptor>(data, 25);
    auto e_data = wtn.to_binary(2565, 5);
    // LOG_INFO << "WhiteNode: " << e_data.to_hex_string();
    auto wtn2 = BwtFS::Node::white_node<RCAEncryptor>(e_data, 5, 2565, wtn.get_start(), wtn.get_length());
    EXPECT_EQ(wtn2.get_index(), 25);
    EXPECT_EQ(wtn2.data().to_hex_string(), data.to_hex_string());
}

TEST(WhiteNode, XOREncryptTest){
    BwtFS::Node::Binary data("Hello world!!!", BwtFS::Node::StringType::ASCII);
    // LOG_INFO << "Binary: " << data.to_hex_string();
    auto wtn = BwtFS::Node::white_node<XOREncryptor>(data, 25);
    auto e_data = wtn.to_binary(25631, 5);
    // LOG_INFO << "WhiteNode: " << e_data.to_hex_string();
    auto wtn2 = BwtFS::Node::white_node<XOREncryptor>(e_data, 5, 25631, wtn.get_start(), wtn.get_length());
    EXPECT_EQ(wtn2.get_index(), 25);
    EXPECT_EQ(wtn2.data().to_hex_string(), data.to_hex_string());
}

TEST(BlackNode, baseTest){
    BwtFS::Node::black_node<void> bkn(10);
    for(int i = 0; i < 10; i++){
        bkn.add_entry(BwtFS::Node::entry(i, BwtFS::Node::NodeType::WHITE_NODE, 1024, 2048, 555, 1));
    }
    auto binary_data = bkn.to_binary();
    // LOG_INFO << "BlackNode: " << binary_data.to_hex_string();
    for (int i = 0; i < 10; i++){
        auto e = bkn.get_entry(i);
        // LOG_INFO << "Entry: " << e.get_bitmap();
        EXPECT_EQ(e.get_type(), BwtFS::Node::NodeType::WHITE_NODE);
        EXPECT_EQ(e.get_start(), 1024);
        EXPECT_EQ(e.get_length(), 2048);
        EXPECT_EQ(e.get_seed(), 555);
        EXPECT_EQ(e.get_level(), 1);
    }
}

TEST(BlackNode, BinaryTest){
    BwtFS::Node::black_node<void> bkn(10);
    for(int i = 0; i < 10; i++){
        bkn.add_entry(BwtFS::Node::entry(i, BwtFS::Node::NodeType::WHITE_NODE, 1024, 2048, 555, 1));
    }
    auto binary_data = bkn.to_binary();
    // LOG_INFO << "BlackNode: " << binary_data.to_hex_string();
    // LOG_INFO << "Start: " << bkn.get_start() << ", Length: " << bkn.get_length();
    auto bkn2 = BwtFS::Node::black_node<void>(binary_data, bkn.get_start(), bkn.get_length());
    EXPECT_EQ(bkn2.get_index(), 10);
    for(int i = 0; i < 10; i++){
        auto e = bkn2.get_entry(i);
        // EXPECT_EQ(e.get_bitmap(), i);
        // LOG_INFO << "Entry: " << e.get_bitmap();
        EXPECT_EQ(e.get_type(), BwtFS::Node::NodeType::WHITE_NODE);
        EXPECT_EQ(e.get_start(), 1024);
        EXPECT_EQ(e.get_length(), 2048);
        EXPECT_EQ(e.get_seed(), 555);
        EXPECT_EQ(e.get_level(), 1);
    }
}

TEST(BlackNode, fromBinaryTest){
    BwtFS::Node::black_node<void> bkn(10);
    int size = 255;
    for(int i = 0; i < size; i++){
        bkn.add_entry(BwtFS::Node::entry(i, BwtFS::Node::NodeType::WHITE_NODE, 1024, 2048, 555, 1));
    }
    auto binary_data = bkn.to_binary();
    // LOG_INFO << "BlackNode: " << binary_data.to_hex_string();
    auto bkn2 = BwtFS::Node::black_node<void>(binary_data, bkn.get_start(), bkn.get_length());
    EXPECT_EQ(bkn2.get_index(), 10);
    for(int i = 0; i < size; i++){
        auto e = bkn2.get_entry(i);
        // EXPECT_EQ(e.get_bitmap(), i);
        // LOG_INFO << "Entry: " << e.get_bitmap();
        EXPECT_EQ(e.get_type(), BwtFS::Node::NodeType::WHITE_NODE);
        EXPECT_EQ(e.get_start(), 1024);
        EXPECT_EQ(e.get_length(), 2048);
        EXPECT_EQ(e.get_seed(), 555);
        EXPECT_EQ(e.get_level(), 1);
    }
}

TEST(BlackNode, encryptTest){
    BwtFS::Node::black_node<RCAEncryptor> bkn(10);
    for(int i = 0; i < 250; i++){
        bkn.add_entry(BwtFS::Node::entry(i, BwtFS::Node::NodeType::WHITE_NODE, 1024, 2048, 555, 1));
    }
    auto binary_data = bkn.to_binary(523, 1);
    // LOG_INFO << "BlackNode: " << binary_data.to_hex_string();
    auto bkn2 = BwtFS::Node::black_node<RCAEncryptor>(binary_data, 1, 523, bkn.get_start(), bkn.get_length());
    EXPECT_EQ(bkn2.get_index(), 10);
    EXPECT_EQ(bkn2.get_size_of_entry(), 250);
    for(int i = 0; i < bkn2.get_size_of_entry(); i++){
        auto e = bkn2.get_entry(i);
        EXPECT_EQ(e.get_type(), BwtFS::Node::NodeType::WHITE_NODE);
        EXPECT_EQ(e.get_start(), 1024);
        EXPECT_EQ(e.get_length(), 2048);
        EXPECT_EQ(e.get_seed(), 555);
        EXPECT_EQ(e.get_level(), 1);
    }
}