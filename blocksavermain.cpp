#include "blocksaver.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

class BlockSaverApp
{
public:
    void Sync(FileManager::AddAndRemoveFileListPair& diffResult, FileManager& sourceManager, FileManager& destManager)
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
                    if(destManager.GetFiles().count(filename) == 0)
                    {
                        auto& loader = sourceManager.GetFiles().at(filename);
                        loader->Load();
                        destManager.AddFile(filename, loader->GiveupData());
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
                    if(destManager.GetFiles().count(filename) > 0)
                    {
                        destManager.RemoveFile(filename);
                        cout  << "  - " << filename << " removed\n";
                    }
                }                
            }
    }

    void Run()
    {
        m_Run = true;
        auto fileOperationManager = new FileDataOperationManager("/home/gani/work/temp/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
        FileManager manager(fileOperationManager);
        manager.SetName("Manager 1");
        auto fileOperationManager2 = new FileDataOperationManager("/home/gani/work/temp2/", {}, {'\n', 'E', 'O', 'F'});
        FileManager manager2(fileOperationManager2);
        manager2.SetName("Manager 2");
        
        while(m_Run)
        {
            auto diffResult = manager.DiffFiles();
            auto diffResult2 = manager2.DiffFiles();
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