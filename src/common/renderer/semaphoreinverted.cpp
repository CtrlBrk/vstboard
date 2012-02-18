#include "semaphoreinverted.h"

SemaphoreInverted::SemaphoreInverted() :
    lockCpt(0)
{
    mutex = new QMutex();
    cond = new QWaitCondition();
}

SemaphoreInverted::~SemaphoreInverted()
{
    delete mutex;
    delete cond;
}

void SemaphoreInverted::AddLock(int n)
{
    QMutexLocker locker(mutex);
    lockCpt+=n;
}

void SemaphoreInverted::Unlock(int n)
{
    QMutexLocker locker(mutex);
    lockCpt-=n;
    if(lockCpt<0) {
        lockCpt=0;
    }
    if(lockCpt==0)
        cond->wakeAll();
}

void SemaphoreInverted::WaitUnlock()
{
    QMutexLocker locker(mutex);
    while(lockCpt>0)
        cond->wait(locker.mutex());
}

bool SemaphoreInverted::IsLocked()
{
    QMutexLocker locker(mutex);
    return (lockCpt!=0);
}

bool SemaphoreInverted::WaitUnlock(int timeout)
{
    QMutexLocker locker(mutex);
    QElapsedTimer timer;
    timer.start();
    while (lockCpt!=0) {
        const qint64 elapsed = timer.elapsed();
        if (timeout - elapsed <= 0 || !cond->wait(locker.mutex(), timeout - elapsed))
            return false;
    }
    return true;
}
