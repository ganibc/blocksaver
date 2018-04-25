#include "gtest/gtest.h"

#include "blocksaver.h"
#include <fstream>
#include <cstdio>

TEST(FileManager, Pool) 
{
    //  cleanup before start testing
    std::remove("../test/filemanagertestdata/temp.empty.txt");

    auto fileOperationManager = new FileDataOperationManager("../test/filemanagertestdata/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
    FileManager manager(fileOperationManager);
    {
        auto diffResult = manager.DiffFiles();
        EXPECT_EQ(diffResult.first.size(), 2);
        EXPECT_EQ(diffResult.second.size(), 0);
    }
    {
        auto iter = manager.GetFiles().find("correct.empty.txt");
        EXPECT_TRUE(iter != manager.GetFiles().end());
    }
    {
        auto iter = manager.GetFiles().find("correct.1.txt");
        EXPECT_TRUE(iter != manager.GetFiles().end());
    }
    {
        std::ofstream ofile("../test/filemanagertestdata/temp.empty.txt");
        ofile << "GBTGBT";
        ofile.close();
        {
            auto diffResult = manager.DiffFiles();
            EXPECT_EQ(diffResult.second.size(), 0);

            auto& newFiles = diffResult.first;
            ASSERT_EQ(newFiles.size(), 1);
            EXPECT_EQ(newFiles[0], "temp.empty.txt");
            auto iter = manager.GetFiles().find("temp.empty.txt");
            EXPECT_TRUE(iter != manager.GetFiles().end());
        }
        {
            std::remove("../test/filemanagertestdata/temp.empty.txt");
            auto diffResult = manager.DiffFiles();
            EXPECT_EQ(diffResult.second.size(), 1);
            EXPECT_EQ(diffResult.first.size(), 0);
            auto iter = manager.GetFiles().find("temp.empty.txt");
            EXPECT_TRUE(iter == manager.GetFiles().end());
        }
    }    
}

TEST(FileManager, Data)
{
    auto fileOperationManager = new FileDataOperationManager("../test/filemanagertestdata/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
    FileManager manager(fileOperationManager);
    auto newFiles = manager.DiffFiles();
    {
        const auto& loader = manager.GetFiles().at("correct.1.txt");
        std::string compareStr = "[somestring]";
        EXPECT_TRUE(loader->GetData() == std::vector<char>(compareStr.begin(), compareStr.end()));
    }    
}