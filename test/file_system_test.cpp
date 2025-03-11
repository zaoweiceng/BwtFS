#include "file_system.hpp"
#include "gtest/gtest.h"

TEST(FileSystemTest, CreateFile) {
    BwtFS::FileSystem fs;
    EXPECT_EQ (fs.create_file("/tmp/test.txt"), true);
    EXPECT_EQ (fs.create_file("/tmp/test.txt"), true);
}