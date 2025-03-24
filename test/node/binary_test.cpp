#include "node/binary.h"
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
    auto datas1 = std::vector<std::byte>{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}};
    auto datas2 = std::vector<std::byte>{std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}};
    auto datas3 = std::vector<std::byte>{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}};
    BwtFS::Node::Binary binary1(datas1);
    BwtFS::Node::Binary binary2(datas2);
    BwtFS::Node::Binary binary(datas3);
    EXPECT_FALSE(binary1 == binary2);
    BwtFS::Node::Binary binary3 = binary1 + binary2;
    // std::cout << binary1.to_string() << std::endl;
    // std::cout << binary.to_string() << std::endl;
    // std::cout << binary3.to_string() << std::endl;
    EXPECT_TRUE(binary == binary3);
}

TEST(BinaryTest, String){
    std::string str = "00fff191";
    auto bin = BwtFS::Node::Binary(str);
    // std::cout << bin.to_string() << std::endl;
    EXPECT_EQ(str, bin.to_hex_string());
}

TEST(BinaryTest, WriteData){
    BwtFS::Node::Binary binary(4);
    binary.write(std::vector<std::byte>{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}});
    EXPECT_EQ(binary.to_hex_string(), "01020304");
    binary.set(1, std::byte{5});
    EXPECT_EQ(binary.to_hex_string(), "01050304");
}

TEST(BinaryTest, ReadData){
    BwtFS::Node::Binary binary(4);
    binary.write(std::vector<std::byte>{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}});
    auto data = binary.read(1, 2);
    EXPECT_EQ(data[0], std::byte{2});
    EXPECT_EQ(data[1], std::byte{3});
    auto data1 = binary.get(3);
    EXPECT_EQ(data1, std::byte{4});
}

TEST(BinaryTest, Resize){
    BwtFS::Node::Binary binary(1);
    binary = binary.resize(2);
    EXPECT_EQ(binary.size(), 2);
    EXPECT_EQ(binary.to_hex_string(), "0000");
}

TEST(BinaryTest, Clear){
    BwtFS::Node::Binary binary(4);
    binary.write(std::vector<std::byte>{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}});
    binary.clear();
    EXPECT_EQ(binary.to_hex_string(), "");
}

TEST(BinaryTest, Append){
    BwtFS::Node::Binary binary(4);
    binary.write(std::vector<std::byte>{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}});
    binary.append(std::vector<std::byte>{std::byte{5}, std::byte{6}});
    EXPECT_EQ(binary.to_hex_string(), "010203040506");
}

TEST(BinaryTest, Xor){
    auto datas1 = std::vector<std::byte>{std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}};
    auto datas2 = std::vector<std::byte>{std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}};
    BwtFS::Node::Binary binary1(datas1);
    BwtFS::Node::Binary binary2(datas2);
    BwtFS::Node::Binary binary3 = binary1 ^ binary2;
    BwtFS::Node::Binary binary4 = binary3 ^ binary2;
    EXPECT_EQ(binary1.to_hex_string(), binary4.to_hex_string());
}

TEST(BinaryTest, Ascii){
    std::string str = "hello";
    auto bin = BwtFS::Node::Binary(str, BwtFS::Node::StringType::ASCII);
    EXPECT_EQ(str, bin.to_ascll_string());
}

TEST(BinaryTest, Base64){
    auto bin = BwtFS::Node::Binary("hello world!!!", BwtFS::Node::StringType::ASCII);
    auto base64 = bin.to_base64_string();
    auto bin1 = BwtFS::Node::Binary(base64, BwtFS::Node::StringType::BASE64);
    EXPECT_EQ(bin.to_ascll_string(), bin1.to_ascll_string());
}