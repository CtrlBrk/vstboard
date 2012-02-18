/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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

#include "renderer.h"
#include "pathsolver.h"
#include "connectables/object.h"
#include "solvernode.h"
#include "mainhost.h"
#include "views/configdialog.h"
#include "renderernode.h"

Renderer::Renderer(MainHost *myHost)
    : QObject(),
      numberOfThreads(0),
      numberOfSteps(0),
      stop(false),
      newNodes(false),
      needOptimize(false),
      nextOptimize(0),
      needBuildModel(false),
      sem(0),
      myHost(myHost)
{
    maxNumberOfThreads = ConfigDialog::defaultNumberOfThreads(myHost->settings);
    InitThreads();

    connect(&updateViewTimer, SIGNAL(timeout()),
            this, SLOT(UpdateView()));
    updateViewTimer.start(500);
}

Renderer::~Renderer()
{
    updateViewTimer.stop();
    Clear();
}

void Renderer::UpdateView()
{
    if(!mutex.tryLockForRead())
        return;

    if(needBuildModel) {
        needBuildModel=false;

        MsgObject msg(FixedObjId::mainWindow);
        msg.prop[MsgObject::SolverMap]=1;

        for(int th=0; th<maxNumberOfThreads; th++) {
            QMap<int, RendererNode* >lst = optimizer.GetThreadNodes(th);

            MsgObject msgThread(th);
            QMap<int, RendererNode* >::iterator i = lst.begin();
            while(i!=lst.end()) {
                MsgObject msgStep(i.key());
                i.value()->GetInfo(msgStep);
                msgThread.children << msgStep;
                ++i;
            }
            msg.children << msgThread;
        }
        msg.prop[MsgObject::Row]=numberOfSteps;
        msg.prop[MsgObject::Col]=maxNumberOfThreads;
        myHost->SendMsg(msg);

//        optimizer.BuildModel( &model );
//        optimizer.UpdateView( &model );
    }

//    int i = 0;
//    foreach(RenderThread *th, listOfThreads) {
//        model.setHorizontalHeaderItem(i, new QStandardItem( QString("%1 (cpu:%2)").arg(i).arg(th->currentCpu) ));
//        ++i;
//    }
    mutex.unlock();
}

//QStandardItemModel * Renderer::GetModel()
//{
//    return &model;
//}

void Renderer::Clear()
{
    mutex.lockForWrite();
    stop=true;
    numberOfThreads=0;
    numberOfSteps=0;
    qDeleteAll(listOfThreads);
    listOfThreads.clear();
    mutex.unlock();
}

void Renderer::SetNbThreads(int nbThreads)
{
    if(nbThreads<=0) nbThreads = 2;

    mutex.lockForWrite();
    qDeleteAll(listOfThreads);
    listOfThreads.clear();
    maxNumberOfThreads = nbThreads;
    InitThreads();
    mutex.unlock();
}

void Renderer::LoadNodes(const QList<RendererNode*> & listNodes)
{
    if(listNodes.isEmpty()) {
        myHost->SetSolverUpdateNeeded();
        return;
    }

    mutexNodes.lock();
    //copy nodes
    QList<RendererNode*> tmpListOfNodes;
    foreach(RendererNode *n, listNodes) {
        tmpListOfNodes << new RendererNode(*n);
    }
    optimizer.NewListOfNodes( tmpListOfNodes );
    newNodes=true;
    mutexNodes.unlock();

    mutexOptimize.lock();
    nextOptimize=0;
    needOptimize=true;
    mutexOptimize.unlock();
}

QList<RendererNode*> Renderer::SaveNodes()
{
    return optimizer.GetListOfNodes();
}

void Renderer::OnNewRenderingOrder(const QList<SolverNode*> & listNodes)
{
    mutexNodes.lock();

    //copy nodes
    QList<RendererNode*> tmpListOfNodes;
    foreach(const SolverNode *n, listNodes) {
        tmpListOfNodes << new RendererNode(*n);
    }

    optimizer.SetNbThreads(maxNumberOfThreads);
    optimizer.NewListOfNodes( tmpListOfNodes );

    newNodes=true;
    mutexNodes.unlock();

    if(maxNumberOfThreads>1) {
        mutexOptimize.lock();
        nextOptimize=50;
        QTimer::singleShot(nextOptimize, this, SLOT(Optimize()));
        mutexOptimize.unlock();
    }
}

void Renderer::StartRender()
{
    if(!mutex.tryLockForRead(5)) {
        LOG("can't lock renderer");
        return;
    }

    if(stop) {
        mutex.unlock();
        return;
    }

    mutexNodes.lock();
    if(newNodes) {
        newNodes=false;
        mutexNodes.unlock();
        GetStepsFromOptimizer();
        needBuildModel=true;
    } else {
        mutexNodes.unlock();
    }

    mutexOptimize.lock();
    if(needOptimize) {
        needOptimize=false;
        mutexOptimize.unlock();

        QList<RendererNode*> tmpListOfNodes;
        foreach(RenderThread *th, listOfThreads) {
            tmpListOfNodes << th->GetListOfNodes();
        }

        optimizer.NewListOfNodes( tmpListOfNodes );
        optimizer.Optimize();
        GetStepsFromOptimizer();
        needBuildModel=true;
    } else {
        mutexOptimize.unlock();
    }

    if(numberOfThreads<=0) {
        mutex.unlock();
        return;
    }

    int maxRenderingTime = myHost->GetSampleRate()/myHost->GetBufferSize();
    int renderTimeout = 2000;

    if(sem.available()!=0) {
        LOG("semaphore available before rendering !"<<sem.available());
        sem.acquire(sem.available());
    }

    sem.release(maxNumberOfThreads);
    for(int currentStep=-1; currentStep<numberOfSteps; currentStep++) {

        if(sem.tryAcquire(maxNumberOfThreads,maxRenderingTime)) {
            foreach( RenderThread *th, listOfThreads) {
                th->StartRenderStep( currentStep );
            }
        } else if( sem.tryAcquire(maxNumberOfThreads,renderTimeout) ) {
            LOG("renderer step"<<currentStep-1<<" took more than the total available time ("<<maxRenderingTime<<"ms) , finished threads:"<< sem.available() << "/" << maxNumberOfThreads);
            foreach( RenderThread *th, listOfThreads) {
                th->StartRenderStep( currentStep );
            }
        } else {
            LOG("renderer step"<<currentStep-1<<"timeout, finished threads:"<< sem.available() << "/" << maxNumberOfThreads);
            sem.acquire(sem.available());
            mutex.unlock();
            return;
        }
    }

    //wait for the last step to finish
    if(sem.tryAcquire(maxNumberOfThreads,maxRenderingTime)) {
        mutex.unlock();
        return;
    } else if(sem.tryAcquire(maxNumberOfThreads,renderTimeout) ) {
        LOG("renderer last step took more than the total available time ("<<maxRenderingTime<<"ms) , finished threads:"<< sem.available() << "/" << maxNumberOfThreads);
    } else {
        LOG("renderer last step timeout, finished threads:"<< sem.available() << "/" << maxNumberOfThreads);
        sem.acquire(sem.available());
    }
    mutex.unlock();
}

void Renderer::Optimize()
{
    mutexOptimize.lock();
    needOptimize=true;

    switch(nextOptimize) {
        case 50:
            nextOptimize=500;
            break;
        case 500:
            nextOptimize=5000;
            break;
        default:
            nextOptimize=0;
            break;
    }

    if(nextOptimize!=0)
        QTimer::singleShot(nextOptimize, this, SLOT(Optimize()));

    mutexOptimize.unlock();
}

void Renderer::GetStepsFromOptimizer()
{
    numberOfSteps = -1;
    numberOfThreads = maxNumberOfThreads;

//    LOG("new order")

    for(int th=0; th<maxNumberOfThreads; th++) {
        QMap<int, RendererNode* >lst = optimizer.GetThreadNodes(th);

        RenderThread *thread = listOfThreads.value(th);
        thread->SetListOfSteps( lst );

        if(!lst.isEmpty()) {
            int lastStep = lst.uniqueKeys().last();
            if( lastStep >= numberOfSteps) {
                numberOfSteps = lastStep+1;
            }
        }

//#ifndef QT_NO_DEBUG
//        QList<int>usedSteps;
//        QMap<int, RendererNode* >::iterator i = lst.begin();
//        while(i!=lst.end()) {
//            int start = i.value()->minRenderOrder;
//            int end = i.value()->maxRenderOrder;

//            LOG( "th" << th << "step" << i.key() << "[" << start << end << "] (was " << i.value()->minRenderOrderOri << i.value()->maxRenderOrderOri << ")" )

//            for(int j=start; j<=end; j++) {
//                if(usedSteps.contains(j)) {
//                    LOG("overlap"<<j)
//                } else {
//                    usedSteps << j;
//                }
//            }


//            ++i;
//        }
//#endif
    }
}

void Renderer::InitThreads()
{
    for(int i=0; i<maxNumberOfThreads; i++) {
        RenderThread *th = new RenderThread(this, i, QString::number(i));
        listOfThreads << th;
        th->start(QThread::TimeCriticalPriority);
    }
}

