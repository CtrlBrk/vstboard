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
    bool EndPointWait(int timeout);
    void EndPoint();
    void AddClient();
    void RemoveClient();

private:
    bool AtStart(int timeout);
    bool AtEnd(int timeout);
    QMutex mutex;
    QMutex mutexRun;
    QWaitCondition condRunning;
    QWaitCondition condStart;

    volatile int cptAtStart;
    int cptRunning;
 };

#endif // WAITALL_H

