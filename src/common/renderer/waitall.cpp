#include "waitall.h"


WaitAll::WaitAll() :
    cptAtStart(0)
{
}

WaitAll::~WaitAll()
{
    QMutexLocker l(&mutex);
    condStart.wakeAll();
}

void WaitAll::AddClient()
{
    QMutexLocker l(&mutex);
    ++cptAtStart;
}

void WaitAll::WakeAll()
{
    condStart.wakeAll();
}

void WaitAll::RemoveClient()
{
    QMutexLocker l(&mutex);
    --cptAtStart;
    if(cptAtStart<=0) {
        condStart.wakeAll();
    }
}

bool WaitAll::IsReady()
{
    QMutexLocker l(&mutex);
    return (cptAtStart<=1);
}

bool WaitAll::WaitAllThreads(int timeout)
{
    QMutexLocker l(&mutex);


    if(cptAtStart<=1) {
        cptAtStart=1;
        condStart.wakeAll();
        return true;
    }

    --cptAtStart;

    if(!condStart.wait(l.mutex(),timeout)) {
        ++cptAtStart;
        condStart.wakeAll();
        return false;
    }

    ++cptAtStart;
    return true;
}

