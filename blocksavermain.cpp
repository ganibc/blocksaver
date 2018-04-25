#include "blocksaver.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;

class BitcoinOutputMonitor
{
public:
    void Run()
    {
        m_Run = true;
        auto fileOperationManager = new FileDataOperationManager("/home/gani/work/temp/", {'G', 'B', 'T'}, {'G', 'B', 'T'});
        FileManager manager(fileOperationManager);
        
        while(m_Run)
        {
            auto diffResult = manager.DiffFiles();
            if(!diffResult.first.empty())
            {
                cout << "New files:\n";
                for(auto& filename : diffResult.first)
                {
                    cout  << "  - " << filename << "\n";
                }
            }
            if(!diffResult.second.empty())
            {
                cout << "Removed files:\n";
                for(auto& filename : diffResult.second)
                {
                    cout  << "  - " << filename << "\n";
                }
            }

            this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    bool m_Run;
};

int main()
{
    BitcoinOutputMonitor bitcoinMonitor;

    thread btcMonThread(&BitcoinOutputMonitor::Run, &bitcoinMonitor);

    btcMonThread.join();

    return 0;
}