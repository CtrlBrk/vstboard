#include "rendererthread2.h"
#include "renderer2.h"
#include "renderernode2.h"
#include "semaphoreinverted.h"
#include "waitall.h"

RendererThread2::RendererThread2(Renderer2 *renderer, int id) :
    QThread(renderer),
    renderer(renderer),
    stop(false),
    id(id)
{
    renderer->waitThreadReady.AddClient();
    renderer->waitThreadEnd.AddClient();

#ifdef WIN32
    DllAvRt = NULL;
    FunctionAvSetMmThreadCharacteristics = NULL;
    FunctionAvRevertMmThreadCharacteristics = NULL;
    FunctionAvSetMmThreadPriority = NULL;

    DllAvRt = LoadLibrary(TEXT("avrt.dll"));
    if (DllAvRt != NULL)
    {
        FunctionAvSetMmThreadCharacteristics = (AVSETMMTHREADCHARACTERISTICS*)GetProcAddress(DllAvRt,"AvSetMmThreadCharacteristicsA");
        FunctionAvRevertMmThreadCharacteristics = (AVREVERTMMTHREADCHARACTERISTICS*)GetProcAddress(DllAvRt, "AvRevertMmThreadCharacteristics");
        FunctionAvSetMmThreadPriority = (AVSETMMTHREADPRIORITY*)GetProcAddress(DllAvRt, "AvSetMmThreadPriority");
    }

    /* If we have access to AVRT.DLL (Vista and later), use it */
    if (FunctionAvSetMmThreadCharacteristics != NULL) {
        DWORD dwTask = 0;
        HANDLE hMmTask = FunctionAvSetMmThreadCharacteristics(QString("vstboard%1").arg(id).toStdString().c_str(), &dwTask);
        if (hMmTask != NULL && hMmTask != INVALID_HANDLE_VALUE) {
            BOOL bret = FunctionAvSetMmThreadPriority(hMmTask, PA_AVRT_PRIORITY_CRITICAL);
            if (!bret) {
                LOG("can't set msc priority");
            }
        }
        else {
            LOG("can't set msc priority, avrt.dll not loaded");
        }
    }
#endif

    start(QThread::TimeCriticalPriority);
}

RendererThread2::~RendererThread2()
{


    Stop();

    while(isRunning()) {
        Sleep(50);
    }


#ifdef WIN32
    if (hMmTask != NULL) {
        FunctionAvRevertMmThreadCharacteristics(hMmTask);
    }
    if(DllAvRt)
        FreeLibrary(DllAvRt);
#endif

    LOG("thread"<<id<<"deleted")
}

void RendererThread2::Stop()
{
    QMutexLocker locker(&mutexStop);
    stop=true;
    LOG("thread"<<id<<"stopped")
}

bool RendererThread2::IsStopped()
{
    QMutexLocker locker(&mutexStop);
    return stop;
}

void RendererThread2::run()
{
    {
        QMutexLocker locker(&mutexStop);
        stop=false;
    }

    forever {

        if(!renderer->waitThreadReady.WaitAllThreads(1000)) {
            LOG("thread"<<id<<"start timeout")
//            QMutexLocker locker(&mutexStop);
//            stop=true;
            emit Timeout();
//            return;
        }
        {
            QMutexLocker locker(&mutexStop);
            if(stop) {
                renderer->nbThreads--;
                renderer->stepCanStart.first()->Unlock();
                renderer->waitThreadReady.RemoveClient();
                renderer->waitThreadEnd.RemoveClient();
                return;
            }
        }

        {
            QMutexLocker locker(&mutexNewNodes);

            if(!currentNodes.isEmpty()) {

                LockAllSteps();
                renderer->stepCanStart.first()->Unlock();

                if(!renderer->stepCanStart.first()->WaitUnlock(1000)) {
                    LOG("thread"<<id<<"1st step timeout")
//                    emit Timeout();
                }

                ThreadNodes::const_iterator i = currentNodes.constBegin();
                while(i!=currentNodes.constEnd()) {
                    foreach(const QSharedPointer<RendererNode2> &n, i.value()) {
                        n->startSemaphore->WaitUnlock();
                        n->Render();
                        n->nextStepSemaphore->Unlock();
                    }

                    ++i;
                }
            } else {
                renderer->stepCanStart.first()->Unlock();
            }
        }

        if(!renderer->waitThreadEnd.WaitAllThreads(1000)) {
            LOG("thread"<<id<<"end timeout")
//            QMutexLocker locker(&mutexStop);
//            stop=true;
//            renderer->nbThreads--;
//            renderer->waitThreadReady.RemoveClient();
//            renderer->waitThreadEnd.RemoveClient();
//            return;
//            emit Timeout();
        }
    }
}

void RendererThread2::LockAllSteps()
{
    ThreadNodes::iterator step = currentNodes.begin();
    while(step != currentNodes.end()) {
        foreach(const QSharedPointer<RendererNode2> &node, step.value()) {
            node->nextStepSemaphore->AddLock();
            if(node)
                node->NewRenderLoop();
        }
        ++step;
    }
}

void RendererThread2::SetListOfNodes(const ThreadNodes &n)
{
    QMutexLocker locker(&mutexNewNodes);
    currentNodes = n;
}

void RendererThread2::ResetNodes()
{
    QMutexLocker locker(&mutexNewNodes);
    currentNodes.clear();
}
