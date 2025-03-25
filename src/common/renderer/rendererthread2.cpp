/**************************************************************************
#    Copyright 2010-2025 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifdef __linux__
    #include <ctime>
#endif

#include "rendererthread2.h"
#include "renderer2.h"
#include "renderernode2.h"
#include "semaphoreinverted.h"
#include "waitall.h"

RendererThread2::RendererThread2(Renderer2 *renderer, int id) :
    QThread(renderer),
    stop(false),
    stopped(false),
    renderer(renderer),
    id(id)
#ifdef WIN32
	, hMmTask(0)
#endif
{
    setObjectName("rendererThread");
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
#endif
    start(QThread::TimeCriticalPriority);
}

RendererThread2::~RendererThread2()
{
    Stop();

    while(!stopped) {
#ifdef WIN32
        Sleep(50);
#else
        struct timespec r,m;
        r.tv_sec = 0;
        r.tv_nsec = 50 * 1000 * 1000;
        nanosleep(&r,&m);
#endif
    }

#ifdef WIN32
    if (hMmTask != NULL) {
        FunctionAvRevertMmThreadCharacteristics(hMmTask);
    }
    if(DllAvRt)
        FreeLibrary(DllAvRt);
#endif

#ifdef DEBUG_RENDERER
    LOG("thread"<<id<<"deleted")
#endif
}

void RendererThread2::Stop()
{
    stop=true;
#ifdef DEBUG_RENDERER
//    LOG("thread"<<id<<"stopped")
#endif
}

// bool RendererThread2::IsStopped()
// {
//     return stop;
// }

void RendererThread2::run()
{
#ifdef WIN32
    /* If we have access to AVRT.DLL, use it */
    if (FunctionAvSetMmThreadCharacteristics != NULL) {
        DWORD dwTask = 0;
        hMmTask = FunctionAvSetMmThreadCharacteristics("Pro Audio", &dwTask);
        if (hMmTask != NULL && hMmTask != INVALID_HANDLE_VALUE) {
            BOOL bret = FunctionAvSetMmThreadPriority(hMmTask, PA_AVRT_PRIORITY_CRITICAL);
            if (!bret) {
                LOG(QString("can't set msc priority %1").arg(GetLastError()));
            }
        } else {
            LOG(QString("can't set msc priority, hMmTask null %1").arg(GetLastError()));
        }

    } else {
        LOG("can't set msc priority, avrt.dll not loaded");
    }
#endif

    stop=false;

    forever {

        if(!renderer->waitThreadReady.WaitAllThreads(1000)) {
#ifdef DEBUG_RENDERER
            LOG("thread"<<id<<"start timeout")
#endif
//            QMutexLocker locker(&mutexStop);
//            stop=true;
//            emit Timeout();
//            return;
        }

        if(stop) {
//                renderer->nbThreads--;
            renderer->stepCanStart.first()->Unlock();
            renderer->waitThreadReady.RemoveClient();
            renderer->waitThreadEnd.RemoveClient();
            stopped=true;
            return;
        }

        {
            QMutexLocker locker(&mutexNewNodes);

            if(!currentNodes.isEmpty()) {

                LockAllSteps();
                renderer->stepCanStart.first()->Unlock();

                if(!renderer->stepCanStart.first()->WaitUnlock(1000)) {
#ifdef DEBUG_RENDERER
//                    LOG("thread"<<id<<"1st step timeout")
#endif
//                    return;
//                    emit Timeout();
//                    stop=true;
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
#ifdef DEBUG_RENDERER
//            LOG("thread"<<id<<"end timeout")
#endif
//            QMutexLocker locker(&mutexStop);
//            stop=true;
//            renderer->nbThreads--;
//            renderer->waitThreadReady.RemoveClient();
//            renderer->waitThreadEnd.RemoveClient();
//            return;
//            emit Timeout();
        }
    }

    stopped=true;
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
