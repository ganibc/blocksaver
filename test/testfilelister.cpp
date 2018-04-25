#include "gtest/gtest.h"

#include "blocksaver.h"
#include <algorithm>

TEST(FileLister, _1File) 
{
    FileDataOperationManager lister("../test/filelistertestdata/dir1", {}, {});    
    auto files = lister.GetFileList();
    EXPECT_EQ(files.size(), 1);
    //  check filenames
    EXPECT_NE(std::find(files.begin(), files.end(), "file1.txt"), files.end());
    //  check not exist
    EXPECT_EQ(std::find(files.begin(), files.end(), "file2.txt"), files.end());
}

TEST(FileLister, _2File) 
{
    FileDataOperationManager lister("../test/filelistertestdata/dir2", {}, {});    
    auto files = lister.GetFileList();
    EXPECT_EQ(files.size(), 2);
    //  check filenames
    EXPECT_NE(std::find(files.begin(), files.end(), "file1.txt"), files.end());
    EXPECT_NE(std::find(files.begin(), files.end(), "file2.txt"), files.end());
    //  check not exist
    EXPECT_EQ(std::find(files.begin(), files.end(), "file3.txt"), files.end());
}

TEST(FileLister, _3File) 
{
    FileDataOperationManager lister("../test/filelistertestdata/dir3", {}, {});    
    auto files = lister.GetFileList();
    EXPECT_EQ(files.size(), 3);
    //  check filenames
    EXPECT_NE(std::find(files.begin(), files.end(), "file1.txt"), files.end());
    EXPECT_NE(std::find(files.begin(), files.end(), "file2.txt"), files.end());
    EXPECT_NE(std::find(files.begin(), files.end(), "file3.txt"), files.end());
    //  check not exist
    EXPECT_EQ(std::find(files.begin(), files.end(), "file4.txt"), files.end());
}

TEST(FileLister, RegexDigitOnly) 
{
    FileDataOperationManager lister("../test/filelistertestdata/regex", {}, {});    
    auto files = lister.GetFileList(std::regex("(\\d+).txt"));
    ASSERT_EQ(files.size(), 2);
    //  check filenames
    EXPECT_NE(std::find(files.begin(), files.end(), "12345.txt"), files.end());
    EXPECT_NE(std::find(files.begin(), files.end(), "112233.txt"), files.end());
}

TEST(FileLister, RegexNonDigitOnly) 
{
    FileDataOperationManager lister("../test/filelistertestdata/regex", {}, {});    
    auto files = lister.GetFileList(std::regex("(\\D+).txt"));
    ASSERT_EQ(files.size(), 1);
    //  check filenames
    EXPECT_NE(std::find(files.begin(), files.end(), "aabbcc.txt"), files.end());
}

TEST(FileLister, RegexContainBothOnly) 
{
    FileDataOperationManager lister("../test/filelistertestdata/regex", {}, {});    
    auto files = lister.GetFileList(std::regex("(\\D+)(\\d+).txt"));
    ASSERT_EQ(files.size(), 2);
    //  check filenames
    EXPECT_NE(std::find(files.begin(), files.end(), "file1.txt"), files.end());
    EXPECT_NE(std::find(files.begin(), files.end(), "somefile1.txt"), files.end());
}
    
TEST(FileLister, NoDir) 
{
    FileDataOperationManager lister("../test/filelistertestdata/notexist", {}, {});    
    auto files = lister.GetFileList();
    EXPECT_EQ(files.size(), 0);
}