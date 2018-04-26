#include "blocksaver.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

class BlockSaverApp
{
public:
    void Sync(DataManager::AddAndRemoveDataListPair& diffResult, DataManager& sourceManager, DataManager& destManager)
    {
            if(!diffResult.first.empty())
            {
                cout << sourceManager.GetName() << " new files:\n";
                for(auto& filename : diffResult.first)
                {
                    cout  << "  - " << filename << "\n";
                }

                cout << destManager.GetName() << " syncing\n";
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
                cout << sourceManager.GetName() << " removed files:\n";
                for(auto& filename : diffResult.second)
                {
                    cout  << "  - " << filename << "\n";
                }
                cout << destManager.GetName() << " syncing\n";
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

    void Run()
    {
        m_Run = true;
        auto fileOperationManager = new FileDataOperationManager("/home/gani/work/temp/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
        DataManager manager(fileOperationManager);
        manager.SetName("Manager 1");
        auto fileOperationManager2 = new FileDataOperationManager("/home/gani/work/temp2/", {}, {'\n', 'E', 'O', 'F'});
        DataManager manager2(fileOperationManager2);
        manager2.SetName("Manager 2");
        
        while(m_Run)
        {
            auto diffResult = manager.DiffDataHandles();
            auto diffResult2 = manager2.DiffDataHandles();
            Sync(diffResult, manager, manager2);
            Sync(diffResult2, manager2, manager);

            this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    bool m_Run;
};

int main()
{
    BlockSaverApp app;

    thread appThread(&BlockSaverApp::Run, &app);

    appThread.join();

    return 0;
}