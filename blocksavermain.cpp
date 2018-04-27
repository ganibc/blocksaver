#include "blocksaver.h"

int main()
{
    BlockSaverApp app;

    thread appThread(&BlockSaverApp::Run, &app);

    appThread.join();

    return 0;
}