#include "gtest/gtest.h"

#include "blocksaver.h"

#define ABC_XYZ_DIR "../test/filedataoperationmanagertestdata/ABC_XYZ/"
#define metadata_eof_DIR "../test/filedataoperationmanagertestdata/metadata_eof/"

FileDataOperationManager Get_ABC_XYZ_OperationManager()
{
    return FileDataOperationManager({'A', 'B', 'C'}, {'X', 'Y', 'Z'});
}

FileDataOperationManager Get_metadata_eof_OperationManager()
{
    return FileDataOperationManager({'m', 'e', 't', 'a', 'd', 'a', 't', 'a'}, {'e', 'o', 'f'});
}

TEST(FileDataOperationManager, ABC_XYZ_Ready) 
{
    FileDataOperationManager manager = Get_ABC_XYZ_OperationManager();
    EXPECT_TRUE(manager.IsFileReadyToLoad(ABC_XYZ_DIR "correct.txt"));
    EXPECT_TRUE(manager.IsFileReadyToLoad(ABC_XYZ_DIR "correct.empty.txt"));
    EXPECT_FALSE(manager.IsFileReadyToLoad(ABC_XYZ_DIR "wrong.txt"));
    EXPECT_FALSE(manager.IsFileReadyToLoad(ABC_XYZ_DIR "notexist.txt"));

}

TEST(FileDataOperationManager, metadata_eof_Ready) 
{
    FileDataOperationManager manager = Get_metadata_eof_OperationManager();
    EXPECT_TRUE(manager.IsFileReadyToLoad(metadata_eof_DIR "correct.txt"));
    EXPECT_FALSE(manager.IsFileReadyToLoad(metadata_eof_DIR "wrong.txt"));
    EXPECT_FALSE(manager.IsFileReadyToLoad(metadata_eof_DIR "notexist.txt"));
}

void CheckContent(FileDataOperationManager& manager, const std::string& filename, const std::vector<char>& expectedValue)
{
    auto loader = manager.GetFileDataLoader(filename);
    ASSERT_TRUE(loader != nullptr);
    EXPECT_FALSE(loader->IsLoaded());
    loader->Load();
    EXPECT_TRUE(loader->IsLoaded());
    EXPECT_TRUE(loader->Data() == expectedValue);
    std::vector<char> unexpectedValue = expectedValue;
    unexpectedValue.push_back('1');
    EXPECT_FALSE(loader->Data() == unexpectedValue);
    loader->Unload();
    EXPECT_FALSE(loader->IsLoaded());
}

void CheckContent(FileDataOperationManager& manager, const std::string& filename, const std::string& expectedValueStr)
{
    std::vector<char> expectedValue(expectedValueStr.begin(), expectedValueStr.end());
    CheckContent(manager, filename, expectedValue);
}

TEST(FileDataOperationManager, ABC_XYZ_Content) 
{
    FileDataOperationManager manager = Get_ABC_XYZ_OperationManager();
    CheckContent(manager, ABC_XYZ_DIR "correct.txt", "somestringcontent");
    CheckContent(manager, ABC_XYZ_DIR "correct.empty.txt", "");
}

TEST(FileDataOperationManager, metadata_eof_Content) 
{
    FileDataOperationManager manager = Get_metadata_eof_OperationManager();
    CheckContent(manager, metadata_eof_DIR "correct.txt", "12345566");
}

TEST(FileDataOperationManager, ABC_XYZ_Save_Delete) 
{
    FileDataOperationManager manager = Get_ABC_XYZ_OperationManager();
    std::string dataStr = "some input to save";
    {
        //  make sure no file.
        manager.DeleteFile(ABC_XYZ_DIR "temp.txt");
        std::vector<char> data(dataStr.begin(), dataStr.end());
        auto loader = manager.SaveDataToFile(ABC_XYZ_DIR "temp.txt", std::move(data), false);
        ASSERT_TRUE(loader != nullptr);
        EXPECT_TRUE(data.empty());
        EXPECT_TRUE(loader->IsLoaded());
        CheckContent(manager, ABC_XYZ_DIR "temp.txt", dataStr);
        EXPECT_TRUE(manager.DeleteFile(ABC_XYZ_DIR "temp.txt"));
    }
}

TEST(FileDataOperationManager, metadata_eof_Save_Delete) 
{
    FileDataOperationManager manager = Get_metadata_eof_OperationManager();
    std::string dataStr = "some input to save";
    {
        //  make sure no file.
        manager.DeleteFile(metadata_eof_DIR "temp.txt");
        std::vector<char> data(dataStr.begin(), dataStr.end());
        auto loader = manager.SaveDataToFile(metadata_eof_DIR "temp.txt", std::move(data), false);
        ASSERT_TRUE(loader != nullptr);
        EXPECT_TRUE(data.empty());
        EXPECT_TRUE(loader->IsLoaded());
        CheckContent(manager, metadata_eof_DIR "temp.txt", dataStr);
        EXPECT_TRUE(manager.DeleteFile(metadata_eof_DIR "temp.txt"));
    }
}