#include "renderer2.h"
#include "renderernode2.h"
#include "solver/solver.h"
#include "rendererthread2.h"


Renderer2::Renderer2(QObject *parent) :
    QObject(parent)
{
    //the first step is always ready
    stepCanStart << new SemaphoreInverted();
}

Renderer2::~Renderer2()
{
    foreach(RendererThread2 *th, threads) {
        th->Stop();
    }
    renderInProgress.WaitUnlock(1000);
    condStartRender.wakeAll();

    qDeleteAll(threads);
    qDeleteAll(threadsToDelete);
    qDeleteAll(stepCanStart);
}

void Renderer2::SetMap(const RenderMap &map, int nbThreads)
{
    ThreadCleanup();
    ChangeNbOfThreads( nbThreads );

    for(int t=0; t<nbThreads; t++) {
        RendererThread2 *thread = threads[t];
        if(map.contains(t)) {

            QList<int>lstStep = map.value(t).keys();
            while(stepCanStart.count()<=lstStep.last()+1) {
                stepCanStart << new SemaphoreInverted();
            }

            thread->SetListOfNodes(map[t]);
            thread->Suspend(false);
        } else {
            thread->Suspend(true);
        }
    }
}

void Renderer2::StartRender()
{
    if(renderInProgress.IsLocked()) {
        LOG("last rendering not finished, drop buffer")
        return;
    }
    condStartRender.wakeAll();
}

void Renderer2::ChangeNbOfThreads(int newNbThreads)
{
    int currentNbThreads = threads.count();

    //remove unneeded threads, don't wait for deletion rigth now
    while(currentNbThreads > newNbThreads) {
        --currentNbThreads;
        RendererThread2* th = threads.takeLast();
        th->Stop();
        threadsToDelete << th;
    }

    //add threads if needed
    while(currentNbThreads < newNbThreads) {
        ++currentNbThreads;
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
