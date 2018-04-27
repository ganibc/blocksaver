#ifndef _BLOCKSAVER_H_
#define _BLOCKSAVER_H_

#include "dataoperation_file.h"
#include "dataoperation_mysql.h"
#include "datamanager.h"

#include <thread>
#include <chrono>
#include <iostream>
#include <vector>

using namespace std;


class SyncWorker
{
public:
    void DoDiffs();

    std::unique_ptr<DataManager> dataManager;
    std::chrono::duration<double> diffPeriod;

    DataManager::AddAndRemoveDataListPair diffResult;
    std::chrono::time_point<std::chrono::system_clock> lastEndDiff;
};

class SyncManager
{
public:
    SyncManager();
    void Run();
    void AddWorker(DataManager* dataManager, std::chrono::duration<double> diffPeriod);

private:
    void DoDiffs();
    void DoSync();
    void Sync(DataManager::AddAndRemoveDataListPair& diffResult, DataManager& sourceManager, DataManager& destManager);

private:
    std::vector<std::unique_ptr<SyncWorker>> m_workers;
    bool m_KeepRun;
};

class BlockSaverApp
{
public:

    void Run()
    {
        m_Run = true;
        auto fileOperationManager = new FileDataOperationManager("/home/gani/work/temp/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
        auto manager1 = new DataManager(fileOperationManager);
        manager1->SetName("Manager 1");
        auto fileOperationManager2 = new FileDataOperationManager("/home/gani/work/temp2/", {}, {'\n', 'E', 'O', 'F'});
        auto manager2 = new DataManager(fileOperationManager2);
        manager2->SetName("Manager 2");


        MYSQL* conn = mysql_init(NULL);
        mysql_real_connect(conn, "localhost", "root", "Hello@123", "testing", 0, NULL, 0);

        auto mysqlOperationManager = new MysqlDataOperationManager(conn, "filedata");
        auto manager3 = new DataManager(mysqlOperationManager);
        manager2->SetName("Manager mysql");


        m_syncManager.AddWorker(manager1, std::chrono::milliseconds(500));
        m_syncManager.AddWorker(manager2, std::chrono::milliseconds(500));
        m_syncManager.AddWorker(manager3, std::chrono::milliseconds(1000));

        m_syncManager.Run();
    }

    SyncManager m_syncManager;
    bool m_Run;
};

#endif