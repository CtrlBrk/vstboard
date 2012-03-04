#ifndef WAITALL_H
#define WAITALL_H

#include "precomp.h"

class WaitAll
{
public:
    WaitAll();
    ~WaitAll();
    bool IsReady();
    bool WaitAllThreads(int timeout);
    void AddClient();
    void RemoveClient();
    void WakeAll();

private:
    QMutex mutex;
    QWaitCondition condStart;
    volatile int cptAtStart;
 };

#endif // WAITALL_H

