#include "gtest/gtest.h"
#include "util/cell.h"

TEST(CellTest, XOR){
    std::byte b = std::byte{0b11010001};
    BwtFS::Util::Cell::XOR(b);
    BwtFS::Util::Cell::XOR_BACK(b);
    EXPECT_EQ(std::to_integer<int>(b), 0b11010001);
}

TEST(CellTest, SHIFT){
    std::byte b = std::byte{0b11010001};
    BwtFS::Util::Cell::SHIFT(b);
    BwtFS::Util::Cell::SHIFT_BACK(b);
    EXPECT_EQ(std::to_integer<int>(b), 0b11010001);
}

TEST(CellTest, FD){
    std::byte b = std::byte{0b11011001};
    BwtFS::Util::Cell::FD(b);
    BwtFS::Util::Cell::FD_BACK(b);
    EXPECT_EQ(std::to_integer<int>(b), 0b11011001);
}

TEST(CellTest, TD){
    std::byte b = std::byte{'l'};
    BwtFS::Util::Cell::TD(b);
    BwtFS::Util::Cell::TD_BACK(b);
    EXPECT_EQ(std::to_integer<int>(b), 'l');
}

TEST(CellTest, Apply){
    std::byte b = std::byte{'l'};
    int rule = 3;
    BwtFS::Util::Cell::apply(b, rule, true);
    BwtFS::Util::Cell::apply(b, rule, false);
    EXPECT_EQ(std::to_integer<int>(b), 'l');
}


TEST(CellTest, Cell){
    BwtFS::Node::Binary binary("Hello World!AAABBBCCC1234567890", BwtFS::Node::StringType::ASCII);
    BwtFS::Util::Cell cell(1024, binary);
    cell.forward();
    auto data = binary.to_ascll_string();
    cell.backward();
    auto data1 = binary.to_ascll_string();
    EXPECT_EQ("Hello World!AAABBBCCC1234567890", data1);
}

TEST(CellTest, RCell){
    std::string str = "Hello World!";
    BwtFS::Node::Binary binary(str, BwtFS::Node::StringType::ASCII);
    BwtFS::Util::Cell cell(1024, binary);
    cell.forward();
    auto data = binary.to_hex_string();

    BwtFS::Node::Binary binary1(data, BwtFS::Node::StringType::BINARY);
    BwtFS::Util::Cell cell1(1024, binary1);
    cell1.backward();
    auto data1 = binary1.to_ascll_string();
    EXPECT_EQ("Hello World!", data1);
}