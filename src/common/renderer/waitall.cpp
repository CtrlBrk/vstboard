#include "waitall.h"


WaitAll::WaitAll() :
    cptAtStart(0),
    cptRunning(0)
{
}

WaitAll::~WaitAll()
{
    QMutexLocker l(&mutex);
    condStart.wakeAll();
    condRunning.wakeAll();
}

void WaitAll::AddClient()
{
    QMutexLocker l(&mutex);
//    if(loopStarted) {
//        ++cptAtEnd;
//    } else {
        ++cptAtStart;
//    }
}

void WaitAll::RemoveClient()
{
    QMutexLocker l(&mutex);
//    if(loopStarted) {
//        --cptAtEnd;
//    } else {
        --cptAtStart;
//    }
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
//        ++cptRunning;
        condStart.wakeAll();
        return true;
    }

    --cptAtStart;

    if(!condStart.wait(l.mutex(),timeout)) {
        ++cptAtStart;
        return false;
    }

    ++cptAtStart;
//    ++cptRunning;
    return true;
}

bool WaitAll::EndPointWait(int timeout)
{
    QMutexLocker l(&mutex);

    --cptAtStart;
    if(cptAtStart==0) {
        ++cptAtStart;
        ++cptRunning;
        condStart.wakeAll();
        return true;
    }
    if(!condStart.wait(&mutex,timeout)) {
        ++cptAtStart;
        return false;
    }

    QMutexLocker r(&mutexRun);
    ++cptAtStart;
    ++cptRunning;

    while(cptRunning!=1) {
        if(!condRunning.wait(&mutexRun,timeout)) {
            --cptRunning;
            return false;
        }
    }
    --cptRunning;
    return true;
}

void WaitAll::EndPoint()
{
    QMutexLocker l(&mutex);
    --cptRunning;
    condRunning.wakeAll();
}
