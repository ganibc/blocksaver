#include "blocksaver.h"


void SyncWorker::DoDiffs()
{
    auto start = std::chrono::system_clock::now();
    auto lastUpdatedDuration = start - lastEndDiff;
    if(lastUpdatedDuration >= diffPeriod)
    {
        diffResult = dataManager->DiffDataHandles();
        lastEndDiff = std::chrono::system_clock::now();
    }
}

SyncManager::SyncManager()
    : m_KeepRun(false)
{

}

void SyncManager::Run()
{
    m_KeepRun = true;

    //  loop every 500 ms
    std::chrono::milliseconds loopPeriod(500);
    auto lastStart = std::chrono::system_clock::now();            

    //  if re-create threads every loop is not fast enough, then might need to write thread pool
    while(m_KeepRun)
    {
        auto start = std::chrono::system_clock::now();
        DoDiffs();

        DoSync();



        auto end = std::chrono::system_clock::now();
        lastStart = start;

        std::chrono::duration<double> diff = end - start;
        auto sleepTime = loopPeriod - std::chrono::duration_cast<std::chrono::milliseconds>(diff);

        if(sleepTime.count() >= 1)
        {
            this_thread::sleep_for(sleepTime);
        }
    }
}

void SyncManager::AddWorker(DataManager* dataManager, std::chrono::duration<double> diffPeriod)
{
    SyncWorker* worker = new SyncWorker;
    worker->dataManager = std::unique_ptr<DataManager>(dataManager);
    worker->diffPeriod = diffPeriod;
    m_workers.emplace_back(worker);
}

void SyncManager::DoInit()
{
    auto lastStart = std::chrono::system_clock::now() - std::chrono::hours(24);            
    for(auto& worker : m_workers)
    {
        worker->lastEndDiff = lastStart;
    }
    DoDiffs();

}

void SyncManager::DoDiffs()
{
    std::vector<std::unique_ptr<std::thread>> workerThreads;
    for(auto& worker : m_workers)
    {
        workerThreads.push_back(make_unique<std::thread>(&SyncWorker::DoDiffs, worker.get()));
    }
    //  wait until diffs are done
    for(auto& t : workerThreads)
    {
        t->join();
    }
}

void SyncManager::DoSync()
{
    int workerCount = m_workers.size();
    for(int i = 0; i < workerCount - 1; ++i)
    {
        auto& worker1 = m_workers[i];
        auto& manager1 = worker1->dataManager;
        for(int j = i + 1; j < workerCount; ++j )
        {
            auto& worker2 = m_workers[j];
            auto& manager2 = worker2->dataManager;
            Sync(worker1->diffResult, *manager1.get(), *manager2.get());
            Sync(worker2->diffResult, *manager2.get(), *manager1.get());
        }
    }

}

void SyncManager::Sync(DataManager::AddAndRemoveDataListPair& diffResult, DataManager& sourceManager, DataManager& destManager)
{
    if(!diffResult.first.empty() || !diffResult.second.empty())
        cout << destManager.GetName() << " syncing from " << sourceManager.GetName() << "\n";
    if(!diffResult.first.empty())
    {

        for(auto& filename : diffResult.first)
        {
            if(destManager.GetDataHandles().count(filename) == 0)
            {
                auto& loader = sourceManager.GetDataHandles().at(filename);
                loader->Load();
                destManager.AddData(filename, loader->GiveupData());
                cout  << "  - " << filename << " copied\n";
            }
        }
    }
    if(!diffResult.second.empty())
    {
        for(auto& filename : diffResult.second)
        {
            if(destManager.GetDataHandles().count(filename) > 0)
            {
                destManager.RemoveData(filename);
                cout  << "  - " << filename << " removed\n";
            }
        }                
    }
}
