#include "semaphoreinverted.h"

SemaphoreInverted::SemaphoreInverted() :
    lockCpt(0)
{
}

SemaphoreInverted::~SemaphoreInverted()
{
}

void SemaphoreInverted::AddLock(int n)
{
    QWriteLocker locker(&mutex);
    lockCpt+=n;
}

void SemaphoreInverted::Unlock(int n)
{
    QWriteLocker locker(&mutex);
//    if(n==-1) {
//        lockCpt=0;
//    } else {
        lockCpt-=n;
//    }

    if(lockCpt<=0) {
        lockCpt=0;
        condUnlock.wakeAll();
    }
}

void SemaphoreInverted::WaitUnlock()
{
    QReadLocker locker(&mutex);
    if(lockCpt==0)
        return;

    condUnlock.wait(&mutex);
}

bool SemaphoreInverted::IsLocked()
{
    QReadLocker locker(&mutex);
    return (lockCpt!=0);
}

bool SemaphoreInverted::WaitUnlock(int timeout)
{
    QReadLocker locker(&mutex);
    if(lockCpt==0)
        return true;

    QElapsedTimer timer;
    timer.start();
    while (lockCpt!=0) {
        qint64 elapsed = timer.elapsed();
        if (timeout - elapsed <= 0 || !condUnlock.wait(&mutex, timeout - elapsed))
            return false;
    }
    return true;
}

bool SemaphoreInverted::WaitUnlock(int timeout, int &timelocked)
{
    QReadLocker locker(&mutex);
    if(lockCpt==0)
        return true;

    QElapsedTimer timer;
    timer.start();
    while (lockCpt!=0) {
        qint64 elapsed = timer.elapsed();
        if (timeout - elapsed <= 0 || !condUnlock.wait(&mutex, timeout - elapsed))
            return false;
    }
    timelocked=timer.elapsed();
    return true;
}

