#include "gtest/gtest.h"

#include "blocksaver.h"

FileDataOperationManager Get_GBT_GBT_OperationManager()
{
    return FileDataOperationManager("../test/filemanagertestdata/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
}

TEST(FileManager, Pool) 
{
    // FileManager manager(new FileLister(TEST_DATA_DIR));
    // auto newFiles = manager.PoolFiles();
    // EXPECT_EQ(newFiles.size(), 1);
    // auto iter = manager.GetFiles().find(TEST_DATA_DIR "correct.empty.txt");
    // EXPECT_TRUE(iter != manager.GetFiles().end());
}