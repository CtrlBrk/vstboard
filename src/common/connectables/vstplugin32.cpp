#include "vstplugin32.h"

VstPlugin32::VstPlugin32() {
    /*
    HANDLE hMutex;
    hMutex = CreateMutex(NULL, FALSE, L"vstboardLock");
    if (!hMutex) {
        return;
    }

    int cpt=0;
    while (true)
    {
        WaitForSingleObject(hMutex, INFINITE);
        LOG("unlock"<<cpt++)
        QThread::sleep(1);
        ReleaseMutex(hMutex);
    }
*/
}
