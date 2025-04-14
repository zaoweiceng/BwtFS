#include "node/entry.h"
#include "node/black_node.h"
#include "node/white_node.h"
#include "node/gray_node.h"
#include "util/log.h"
#include "gtest/gtest.h"

using BwtFS::Util::Logger;

TEST(WhiteNode, baseTest){
    BwtFS::Node::Binary data("Hello world!!!", BwtFS::Node::StringType::ASCII);
    auto wtn = BwtFS::Node::WhiteNode::initWhiteNode(data);
    EXPECT_EQ(wtn.getLength(), data.size());
    // LOG_INFO << wtn.getLength();
    auto data1 = wtn.toBinary();
    // LOG_INFO << data1.to_ascll_string();
    BwtFS::Node::WhiteNode wtn1(data1);
    auto str = wtn1.read(0, 14).to_ascll_string();
    // LOG_INFO << str;
    EXPECT_EQ(str, "Hello world!!!");
}