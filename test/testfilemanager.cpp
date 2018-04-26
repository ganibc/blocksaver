#include "gtest/gtest.h"

#include "blocksaver.h"
#include <fstream>
#include <cstdio>

TEST(DataManager, Pool) 
{
    //  cleanup before start testing
    std::remove("../test/filemanagertestdata/temp.empty.txt");

    auto fileOperationManager = new FileDataOperationManager("../test/filemanagertestdata/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
    DataManager manager(fileOperationManager);
    {
        auto diffResult = manager.DiffDataHandles();
        EXPECT_EQ(diffResult.first.size(), 2);
        EXPECT_EQ(diffResult.second.size(), 0);
    }
    {
        auto iter = manager.GetDataHandles().find("correct.empty.txt");
        EXPECT_TRUE(iter != manager.GetDataHandles().end());
    }
    {
        auto iter = manager.GetDataHandles().find("correct.1.txt");
        EXPECT_TRUE(iter != manager.GetDataHandles().end());
    }
    {
        std::ofstream ofile("../test/filemanagertestdata/temp.empty.txt");
        ofile << "GBTGBT";
        ofile.close();
        {
            auto diffResult = manager.DiffDataHandles();
            EXPECT_EQ(diffResult.second.size(), 0);

            auto& newFiles = diffResult.first;
            ASSERT_EQ(newFiles.size(), 1);
            EXPECT_EQ(newFiles[0], "temp.empty.txt");
            auto iter = manager.GetDataHandles().find("temp.empty.txt");
            EXPECT_TRUE(iter != manager.GetDataHandles().end());
        }
        {
            std::remove("../test/filemanagertestdata/temp.empty.txt");
            auto diffResult = manager.DiffDataHandles();
            EXPECT_EQ(diffResult.second.size(), 1);
            EXPECT_EQ(diffResult.first.size(), 0);
            auto iter = manager.GetDataHandles().find("temp.empty.txt");
            EXPECT_TRUE(iter == manager.GetDataHandles().end());
        }
    }    
}

TEST(DataManager, Data)
{
    auto fileOperationManager = new FileDataOperationManager("../test/filemanagertestdata/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
    DataManager manager(fileOperationManager);
    auto newFiles = manager.DiffDataHandles();
    {
        const auto& loader = manager.GetDataHandles().at("correct.1.txt");
        std::string compareStr = "[somestring]";
        EXPECT_TRUE(loader->GetData() == std::vector<char>(compareStr.begin(), compareStr.end()));
    }    
}