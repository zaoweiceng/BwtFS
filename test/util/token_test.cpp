#include "node/entry.h"
#include "node/bw_node.h"
#include "node/bt_tree.h"
#include "util/log.h"
#include "gtest/gtest.h"
#include "util/token.h"

using BwtFS::Util::Logger;


TEST(TokenTest, baseTest){
    size_t bitmap = 534013;
    uint16_t start = 1245;
    uint16_t length = 32;
    uint16_t seed = 10;
    uint8_t level = 1;
    Token token(bitmap, start, length, seed, level);
    auto token_str = token.generate_token();
    Token token1(token_str);
    EXPECT_EQ(token.get_bitmap(), token1.get_bitmap());
    EXPECT_EQ(token.get_start(), token1.get_start());
    EXPECT_EQ(token.get_length(), token1.get_length());
    EXPECT_EQ(token.get_seed(), token1.get_seed());
    EXPECT_EQ(token.get_level(), token1.get_level());
}