#include "rendererthread2.h"
#include "renderer2.h"
#include "renderernode2.h"
#include "semaphoreinverted.h"

RendererThread2::RendererThread2(Renderer2 *renderer, int id) :
    QThread(renderer),
    renderer(renderer),
    stop(false),
    suspendAsked(true),
    suspended(true),
    id(id)
{
    mutexNewNodes = new QMutex();
    mutexStop = new QMutex();
    mutexSuspend = new QMutex();
    condSuspend = new QWaitCondition();
    start(QThread::TimeCriticalPriority);
}

RendererThread2::~RendererThread2()
{
    {
        QMutexLocker locker(mutexStop);
        stop=true;
    }

    while(isRunning()) {
        Sleep(50);
    }

    delete mutexNewNodes;
    delete mutexStop;
    delete mutexSuspend;
    delete condSuspend;

    LOG("thread"<<id<<"deleted")
}

void RendererThread2::Stop()
{
    {
        QMutexLocker locker(mutexStop);
        stop=true;
    }
    Suspend(false);
    LOG("thread"<<id<<"stopped")
}

void RendererThread2::run()
{
    {
        QMutexLocker locker(mutexStop);
        stop=false;
    }

    forever {

        WaitIfSuspended();

        if(!WaitNextStart())
            return;

        if(CheckIfStopping())
            return;

        renderer->renderInProgress.AddLock();
        ApplyNewNodes();
        LockAllSteps();
        Render();
        renderer->renderInProgress.Unlock();

        if(CheckIfStopping())
            return;
    }
}

void RendererThread2::Render()
{
   bool canceled=false;

   ThreadNodes::iterator step = currentNodes.begin();
    while(step != currentNodes.end()) {
        foreach(RendererNode2 *node, step.value()) {

            if(!renderer->stepCanStart[node->minRenderOrder]->WaitUnlock(2000)) {
                LOG("timeout thread:"<<id<<"step:"<<step.key())
                canceled=true;
            }

            if(!canceled)
                node->Render();

            renderer->stepCanStart[node->maxRenderOrder+1]->Unlock();
        }
        ++step;
    }
}

void RendererThread2::LockAllSteps()
{
    ThreadNodes::iterator step = currentNodes.begin();
    while(step != currentNodes.end()) {
        foreach(RendererNode2 *node, step.value()) {
            renderer->stepCanStart[node->maxRenderOrder+1]->AddLock();
            node->NewRenderLoop();
        }
        ++step;
    }
}

void RendererThread2::ApplyNewNodes()
{
    QMutexLocker locker(mutexNewNodes);
    if(!newNodes.isEmpty()) {
        currentNodes = newNodes;
        newNodes.clear();
    }
}

bool RendererThread2::CheckIfStopping()
{
    QMutexLocker locker(mutexStop);
    return stop;
}

bool RendererThread2::WaitNextStart()
{
    QMutexLocker l(&renderer->mutexStartRender);
    if(!renderer->condStartRender.wait(&renderer->mutexStartRender, 5000)) {
        LOG("timeout, can't start thread:"<<id)
        return false;
    }
    return true;
}

void RendererThread2::WaitIfSuspended()
{
    QMutexLocker locker(mutexSuspend);
    if(suspendAsked) {
        suspended=true;
        condSuspend->wait(locker.mutex());
        suspended=false;
    }
}

void RendererThread2::Suspend(bool s)
{
    QMutexLocker locker(mutexSuspend);
    if(s==suspendAsked)
        return;

    if(s) {
        LOG("suspend"<<id)
        suspendAsked=true;
    } else {
        LOG("release"<<id)
        suspendAsked=false;
        condSuspend->wakeAll();
    }
}

bool RendererThread2::IsSuspended()
{
    QMutexLocker locker(mutexSuspend);
    return suspended;
}

void RendererThread2::SetListOfNodes(const ThreadNodes &n)
{
    QMutexLocker locker(mutexNewNodes);
    newNodes = n;
}
