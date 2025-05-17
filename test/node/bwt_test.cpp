#include "node/entry.h"
#include "node/bw_node.h"
#include "node/bt_tree.h"
#include "util/log.h"
#include "gtest/gtest.h"
#include "util/secure_ptr.h"

using BwtFS::Util::Logger;


// TEST(TreeTest, getNodeTest){
//     BwtFS::Node::bw_tree tree;
//     BwtFS::Node::Binary data("Hello world!!!", BwtFS::Node::StringType::ASCII);
//     tree.write(reinterpret_cast<char*>(data.data()), data.size());
//     tree.flush();
//     auto binary_data = tree.get_node(0);
//     auto wtn = BwtFS::Node::white_node<void>(binary_data.data,
//          binary_data.start, binary_data.length);
//     EXPECT_EQ(wtn.data().to_hex_string(), data.to_hex_string());
// }

// TEST(TreeTest, mutiNodeTest){
//     BwtFS::Node::bw_tree tree;
//     BwtFS::Node::Binary data("Hello world!!!", BwtFS::Node::StringType::ASCII);
//     for (int i = 0; i < 4096*2; i++){
//         data += BwtFS::Node::Binary("Hello world!!!", BwtFS::Node::StringType::ASCII);
//     }
//     tree.write(reinterpret_cast<char*>(data.data()), data.size());
//     tree.flush();
//     auto binary_data = tree.get_node(0);
//     auto wtn = BwtFS::Node::white_node<void>(binary_data.data,
//          binary_data.start, binary_data.length);
//     auto data1 = BwtFS::Node::Binary(data.read(0, BwtFS::BLOCK_SIZE - sizeof(uint8_t)));
//     EXPECT_EQ(wtn.data().to_hex_string(), data1.to_hex_string());
//     EXPECT_EQ(tree.get_node_count(), int(ceil(float((4096*2*14))/(BwtFS::BLOCK_SIZE - sizeof(uint8_t))))-1);
// }