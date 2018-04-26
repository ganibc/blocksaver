#include "gtest/gtest.h"

#include "blocksaver.h"

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

    std::string valueStr = "some string to insert";
    std::vector<char> value(valueStr.begin(), valueStr.end());
    auto loader = manager.StoreData("someid", std::move(value));
    ASSERT_TRUE(loader != nullptr);

}