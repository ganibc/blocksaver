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
    if(!diffResult.first.empty())
    {
        //  TODO: change cout to Log after merge with btcpool
        cout << "new file detected on " << dataManager->GetName() << "\n";
        for(auto& filename : diffResult.first)
        {
            cout << "- " << filename << "\n";
        }
    }
    if(!diffResult.second.empty())
    {
        //  TODO: change cout to Log after merge with btcpool
        cout << "remove file detected on " << dataManager->GetName() << "\n";
        for(auto& filename : diffResult.second)
        {
            cout << "- " << filename << "\n";
        }
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
    DoInit();

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
    m_Workers.emplace_back(worker);
}

void SyncManager::DoInit()
{
    cout << "First initialize\n";
    auto lastStart = std::chrono::system_clock::now() - std::chrono::hours(24);            
    for(auto& worker : m_Workers)
    {
        worker->lastEndDiff = lastStart;
    }
    DoDiffs();
    DoSync();
    cout << "First initialize done\n";
}

void SyncManager::DoDiffs()
{
    std::vector<std::unique_ptr<std::thread>> workerThreads;
    for(auto& worker : m_Workers)
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
    int workerCount = m_Workers.size();
    for(int i = 0; i < workerCount - 1; ++i)
    {
        auto& worker1 = m_Workers[i];
        auto& manager1 = worker1->dataManager;
        for(int j = i + 1; j < workerCount; ++j )
        {
            auto& worker2 = m_Workers[j];
            auto& manager2 = worker2->dataManager;
            Sync(worker1->diffResult, *manager1.get(), *manager2.get());
            Sync(worker2->diffResult, *manager2.get(), *manager1.get());
        }
    }

    //  cleanup. 
    //  Unload to free memory
    //  clear the results for the next syncs
    for(auto& worker : m_Workers)
    {
        auto& manager = *worker->dataManager.get();
        auto& newFiles = worker->diffResult.first;
        for(auto& filename : newFiles)
        {
            auto loader = manager.GetDataHandler(filename);
            if(loader != nullptr)
            {
                loader->Unload();
            }
        }
        worker->diffResult.first.clear();
        worker->diffResult.second.clear();
    }
}

void SyncManager::Sync(DataManager::AddAndRemoveDataListPair& diffResult, DataManager& sourceManager, DataManager& destManager)
{
    if(!diffResult.first.empty() || !diffResult.second.empty())
        //  TODO: change cout to Log after merge with btcpool
        cout << "try sync from " << sourceManager.GetName() << " to " << destManager.GetName() << "\n";
    if(!diffResult.first.empty())
    {
        for(auto& filename : diffResult.first)
        {
            if(destManager.GetDataHandlers().count(filename) == 0)
            {
                auto loader = sourceManager.GetDataHandler(filename);
                if(loader != nullptr)
                {
                    loader->Load();
                    auto dataSize = loader->GetData().size();
                    destManager.AddData(filename, loader->GiveupData());
                    //  TODO: change cout to Log after merge with btcpool
                    cout << "  - " << filename << " copied. size: " << dataSize << " bytes\n";
                }
            }
        }
    }
    if(destManager.IsSyncDelete() && !diffResult.second.empty())
    {
        for(auto& filename : diffResult.second)
        {
            if(destManager.GetDataHandlers().count(filename) > 0)
            {
                destManager.RemoveData(filename);
                //  TODO: change cout to Log after merge with btcpool
                cout << "  - " << filename << " removed\n";
            }
        }                
    }
}
