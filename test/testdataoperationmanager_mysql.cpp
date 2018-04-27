#include "gtest/gtest.h"
#include "blocksaver.h"
#include "testdataoperationmanager_common.h"

const char* server = "localhost";
const char* user = "root";
const char* password = "Hello@123";
const char* database = "testing";


TEST(MySQLDataOperationManager, GetList)
{
    MYSQL* conn = mysql_init(NULL);
    ASSERT_NE(conn, nullptr);
    ASSERT_NE(mysql_real_connect(conn, server, user, password, database, 0, NULL, 0), nullptr);

    MysqlDataOperationManager manager(conn, "filedata");

    manager.DeleteData("someid");
    std::string valueStr = "some string to insert";
    std::vector<char> value(valueStr.begin(), valueStr.end());
    auto loader = manager.StoreData("someid", std::move(value));
    ASSERT_TRUE(loader != nullptr);
    EXPECT_TRUE(loader->Load());
    std::vector<char> expectedValue(valueStr.begin(), valueStr.end());
    EXPECT_TRUE(loader->Data() == expectedValue);
    expectedValue.push_back('a');
    EXPECT_FALSE(loader->Data() == expectedValue);

    auto dataList = manager.GetDataList();
    ASSERT_EQ(dataList.size(), 1);

    manager.DeleteData("someid");

    mysql_close(conn);
}

//  This is to prepare data based on common test cases (see testdataoperationmanager_common.cpp)
void InitTestData(MYSQL* conn)
{
    MysqlDataOperationManager manager(conn, "filedata");
    ASSERT_NE(manager.StoreData("correct.empty.txt", std::vector<char>()), nullptr);    
    std::string correct_number_txt_str = "12345566";
    ASSERT_NE(manager.StoreData("correct.number.txt", std::vector<char>(correct_number_txt_str.begin(), correct_number_txt_str.end())), nullptr);    
    std::string correct_txt_str = "somestringcontent";
    ASSERT_NE(manager.StoreData("correct.txt", std::vector<char>(correct_txt_str.begin(), correct_txt_str.end())), nullptr);    
}

void RemoveTestData(MYSQL* conn)
{
    MysqlDataOperationManager manager(conn, "filedata");
    EXPECT_TRUE(manager.DeleteData("correct.txt"));
    EXPECT_TRUE(manager.DeleteData("correct.number.txt"));
    EXPECT_TRUE(manager.DeleteData("correct.empty.txt"));
}


TEST(MySQLDataOperationManager, Ready) 
{
    MYSQL* conn = mysql_init(NULL);
    ASSERT_NE(conn, nullptr);
    ASSERT_NE(mysql_real_connect(conn, server, user, password, database, 0, NULL, 0), nullptr);

    InitTestData(conn);

    MysqlDataOperationManager manager(conn, "filedata");
    CheckReadyToLoad(manager);

    RemoveTestData(conn);
}

TEST(MySQLDataOperationManager, Content) 
{
    MYSQL* conn = mysql_init(NULL);
    ASSERT_NE(conn, nullptr);
    ASSERT_NE(mysql_real_connect(conn, server, user, password, database, 0, NULL, 0), nullptr);

    InitTestData(conn);

    MysqlDataOperationManager manager(conn, "filedata");
    CheckContents(manager);

    RemoveTestData(conn);
}

TEST(MySQLDataOperationManager, SaveDelete) 
{
    MYSQL* conn = mysql_init(NULL);
    ASSERT_NE(conn, nullptr);
    ASSERT_NE(mysql_real_connect(conn, server, user, password, database, 0, NULL, 0), nullptr);

    InitTestData(conn);

    MysqlDataOperationManager manager(conn, "filedata");
    CheckSaveDelete(manager);

    RemoveTestData(conn);
}