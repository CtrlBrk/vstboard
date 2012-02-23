#include "renderer2.h"
#include "renderernode2.h"
#include "solver/solver.h"
#include "rendererthread2.h"

Renderer2::Renderer2(QObject *parent) :
    QObject(parent),
    nbThreads(0),
    nbSteps(0)
{
    waitThreadReady.AddClient();
    waitThreadEnd.AddClient();

    stepCanStart << new SemaphoreInverted();
}

Renderer2::~Renderer2()
{
    foreach(RendererThread2 *th, threads) {
        th->Stop();
    }
    waitThreadReady.RemoveClient();
    waitThreadEnd.RemoveClient();

    qDeleteAll(threads);
    qDeleteAll(threadsToDelete);
    qDeleteAll(stepCanStart);
}

void Renderer2::SetMap(const RenderMap &map, int nbThreads)
{
    mutexThreadList.lock();
    ChangeNbOfThreads( nbThreads );
    ThreadCleanup();

    for(int t=0; t<nbThreads; t++) {
        RendererThread2 *thread = threads[t];
        if(map.contains(t)) {
            thread->SetListOfNodes(map[t]);
        } else {
            thread->ResetNodes();
        }
    }
    foreach(const RendererThread2 *thread, threads) {
        foreach(const QList<RendererNode2*> &lst, thread->currentNodes) {
            foreach(RendererNode2 *n, lst) {
                while(stepCanStart.count() <= n->maxRenderOrder+1) {
                    stepCanStart << new SemaphoreInverted();
                }

                n->startSemaphore = stepCanStart[n->minRenderOrder];
                n->nextStepSemaphore = stepCanStart[n->maxRenderOrder+1];
            }
        }
    }

    mutexThreadList.unlock();
}

void Renderer2::StartRender()
{
    mutexThreadList.lock();
    stepCanStart.first()->AddLock(nbThreads);
    mutexThreadList.unlock();

    if(!waitThreadReady.WaitAllThreads(10000)) {
        LOG("renderer start timeout")
    }

    if(!waitThreadEnd.WaitAllThreads(10000)) {
        LOG("renderer end timeout")
    }

}

void Renderer2::ChangeNbOfThreads(int newNbThreads)
{
    //remove crashed threads
    for(int i=nbThreads-1; i>=0; i--) {
        RendererThread2* th = threads[i];
        if(!th->isRunning()) {
            --nbThreads;
            threads.removeAt(i);
            threadsToDelete << th;
        }
    }

    //remove unneeded threads, don't wait for deletion rigth now
    while(threads.count() > newNbThreads) {
        //the thread updates the counter
        //        --nbThreads;
        RendererThread2* th = threads.takeLast();
        th->Stop();
        threadsToDelete << th;
    }

    //add threads if needed
    while(nbThreads < newNbThreads) {
        ++nbThreads;
        threads << new RendererThread2(this,threads.count());
    }
}

void Renderer2::ThreadCleanup()
{
    //delete old threads
    int cpt = threadsToDelete.count();
    for(int i=cpt-1; i>=0; i--) {
        RendererThread2* th = threadsToDelete[i];
        if(!th->isRunning()) {
            threadsToDelete.removeAt(i);
            delete th;
        }
    }
}
