#ifndef SEMAPHOREINVERTED_H
#define SEMAPHOREINVERTED_H

#include "precomp.h"

class SemaphoreInverted
{
public:
    SemaphoreInverted();
    ~SemaphoreInverted();

    void AddLock(int n=1);
    void Unlock(int n=1);

    void WaitUnlock();
    bool IsLocked();
    bool WaitUnlock(int timeout);
    bool WaitUnlock(int timeout, int &timelocked);

private:
//    Q_DISABLE_COPY(SemaphoreInverted)
    QReadWriteLock mutex;
    QWaitCondition condUnlock;
    int lockCpt;
};

#endif // SEMAPHOREINVERTED_H
