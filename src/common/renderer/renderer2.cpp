#include "renderer2.h"
#include "renderernode2.h"
#include "solver.h"
#include "rendererthread2.h"

Renderer2::Renderer2(QObject *parent) :
    QObject(parent),
    nbThreads(0),
    nbSteps(0)
{
    waitThreadReady.AddClient();
    waitThreadEnd.AddClient();

    stepCanStart << new SemaphoreInverted();

    signalTimeoutTimer.setSingleShot(true);
    signalTimeoutTimer.setInterval(100);
    connect(&signalTimeoutTimer, SIGNAL(timeout()),
            this, SIGNAL(Timeout()));
}

Renderer2::~Renderer2()
{
    QMutexLocker l(&mutexThreadList);

    foreach(RendererThread2 *th, threads) {
        th->Stop();
    }

//    if(!waitThreadEnd.WaitAllThreads(2000)) {
//        QString err("closing : renderer end timeout");
//        LOG(err)
//    }

    waitThreadReady.RemoveClient();
    waitThreadEnd.RemoveClient();

    qDeleteAll(threads);
    qDeleteAll(threadsToDelete);
    qDeleteAll(stepCanStart);

}

void Renderer2::SetMap(const RenderMap &rMap, int nbThreads)
{
    QMutexLocker l(&mutexThreadList);
    currentMap = rMap;

    ChangeNbOfThreads( nbThreads );
    ThreadCleanup();

    for(int t=0; t<nbThreads; t++) {
        RendererThread2 *thread = threads[t];
        if(currentMap.map.contains(t)) {
            thread->SetListOfNodes(currentMap.map[t]);
        } else {
            thread->ResetNodes();
        }
    }
    foreach(const RendererThread2 *thread, threads) {
        foreach(const RenderStep &step, thread->currentNodes) {
            foreach( QSharedPointer<RendererNode2> n, step) {
                while(stepCanStart.count() <= n->maxRenderOrder+1) {
                    stepCanStart << new SemaphoreInverted();
                }

                n->startSemaphore = stepCanStart[n->minRenderOrder];
                n->nextStepSemaphore = stepCanStart[n->maxRenderOrder+1];
            }
        }
    }
}

void Renderer2::StartRender()
{
    QMutexLocker l(&mutexThreadList);
    stepCanStart.first()->AddLock(nbThreads);

    if(!waitThreadReady.WaitAllThreads(10000)) {
        QString err("renderer start timeout");
//        waitThreadReady.RemoveClient();
//        waitThreadReady.AddClient();
        LOG(err)
//        ChangeNbOfThreads(0);
//        emit Timeout();
//        return;
    }

    if(!waitThreadEnd.WaitAllThreads(2000)) {
        QString err("renderer end timeout");
//        waitThreadEnd.RemoveClient();
//        waitThreadEnd.AddClient();
        LOG(err)
//        ChangeNbOfThreads(0);
//        emit Timeout();
//        return;
    }

}

void Renderer2::ChangeNbOfThreads(int newNbThreads)
{
    //remove crashed threads
    for(int i=nbThreads-1; i>=0; i--) {
        RendererThread2* th = threads[i];
        if(!th->isRunning() && !th->IsStopped()) {
            LOG("remove crashed thread"<<i)
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
        RendererThread2 *th = new RendererThread2(this,threads.count());
        threads << th;
        connect(th,SIGNAL(Timeout()),
                this,SLOT(OnThreadTimeout()));
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

void Renderer2::OnThreadTimeout()
{
    if(signalTimeoutTimer.isActive())
        return;
    signalTimeoutTimer.start();
}
