#include"binary.h"
#include "gtest/gtest.h"

TEST(BinaryTest, CreateBinary) {
    BwtFS::Node::Binary binary(8);
    EXPECT_EQ (binary[0], std::byte{0});
    EXPECT_EQ (binary[1], std::byte{0});
    EXPECT_EQ (binary[2], std::byte{0});
    EXPECT_EQ (binary[3], std::byte{0});
    EXPECT_EQ (binary[4], std::byte{0});
    EXPECT_EQ (binary[5], std::byte{0});
    EXPECT_EQ (binary[6], std::byte{0});
    EXPECT_EQ (binary[7], std::byte{0});
}

TEST(BinaryTest, AddBinary){
    BwtFS::Node::Binary binary1;
    BwtFS::Node::Binary binary2;

}