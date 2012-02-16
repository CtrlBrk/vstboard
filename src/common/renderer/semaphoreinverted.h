#ifndef SEMAPHOREINVERTED_H
#define SEMAPHOREINVERTED_H

#include "precomp.h"

class SemaphoreInverted
{
public:
    SemaphoreInverted();

    void AddLock(int n=1);
    void Unlock(int n=1);

    void WaitUnlock();
    bool IsLocked();
    bool IsLocked(int timeout);

private:
    Q_DISABLE_COPY(SemaphoreInverted)
    QMutex mutex;
    QWaitCondition cond;
    int lockCpt;
};

#endif // SEMAPHOREINVERTED_H
