#include "gtest/gtest.h"
#include "util/cell.h"

TEST(CellTest, XOR){
    std::byte b = std::byte{0b11010001};
    BwtFS::Util::RCA::XOR(b);
    BwtFS::Util::RCA::XOR_BACK(b);
    EXPECT_EQ(std::to_integer<int>(b), 0b11010001);
}

TEST(CellTest, SHIFT){
    std::byte b = std::byte{0b11010001};
    BwtFS::Util::RCA::SHIFT(b);
    BwtFS::Util::RCA::SHIFT_BACK(b);
    EXPECT_EQ(std::to_integer<int>(b), 0b11010001);
}

TEST(CellTest, FD){
    std::byte b = std::byte{0b11011001};
    BwtFS::Util::RCA::FD(b);
    BwtFS::Util::RCA::FD_BACK(b);
    EXPECT_EQ(std::to_integer<int>(b), 0b11011001);
}

TEST(CellTest, TD){
    std::byte b = std::byte{'l'};
    BwtFS::Util::RCA::TD(b);
    BwtFS::Util::RCA::TD_BACK(b);
    EXPECT_EQ(std::to_integer<int>(b), 'l');
}

TEST(CellTest, Apply){
    std::byte b = std::byte{'l'};
    int rule = 3;
    BwtFS::Util::RCA::apply(b, rule, true);
    BwtFS::Util::RCA::apply(b, rule, false);
    EXPECT_EQ(std::to_integer<int>(b), 'l');
}

TEST(CellTest, Cell){
    BwtFS::Node::Binary binary("Hello World!AAABBBCCC1234567890", BwtFS::Node::StringType::ASCII);
    BwtFS::Util::RCA cell(1024, binary);
    cell.forward();
    auto data = binary.to_ascll_string();
    cell.backward();
    auto data1 = binary.to_ascll_string();
    EXPECT_EQ("Hello World!AAABBBCCC1234567890", data1);
}

TEST(CellTest, RCell){
    std::string str = "Hello World!";
    BwtFS::Node::Binary binary(str, BwtFS::Node::StringType::ASCII);
    BwtFS::Util::RCA cell(1024, binary);
    cell.forward();
    auto data = binary.to_hex_string();

    BwtFS::Node::Binary binary1(data, BwtFS::Node::StringType::BINARY);
    BwtFS::Util::RCA cell1(1024, binary1);
    cell1.backward();
    auto data1 = binary1.to_ascll_string();
    // std::cout << data1 << std::endl;
    EXPECT_EQ("Hello World!", data1);
}