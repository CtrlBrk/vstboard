/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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

Renderer::Renderer(MainHost *myHost)
    : QObject(),
      numberOfThreads(0),
      numberOfSteps(0),
      stop(false),
      sem(0),
      myHost(myHost)
{
    maxNumberOfThreads = myHost->GetSetting("MaxNbThreads", 4).toInt();
    if(maxNumberOfThreads<=0) maxNumberOfThreads = 4;
    InitThreads();
}

Renderer::~Renderer()
{
    Clear();
}

void Renderer::InitThreads()
{
    for(int i=0; i<maxNumberOfThreads; i++) {
        RenderThread *th = new RenderThread(this, QString::number(i));
        listOfThreads << th;
        th->start(QThread::TimeCriticalPriority);
    }
}

void Renderer::UpdateView()
{
    foreach(RenderThread *th, listOfThreads) {
        foreach(SolverNode *node, th->listOfSteps) {
            if(node) {
                node->UpdateModel(&model);
            }
        }
    }
}

void Renderer::Optimize()
{
    foreach(RenderThread *th, listOfThreads) {
        th->cpuTotal=0;
        foreach(SolverNode *node, th->listOfSteps) {
            if(node) {
                th->cpuTotal+=node->cpuTime;
            }
        }
    }

    unsigned long maxThreadTime=0;
    unsigned long maxNodeTime=0;
    RenderThread *biggestThreadWithMergedNode=0;
    SolverNode *nodeToMove=0;

    foreach(RenderThread *th, listOfThreads) {
        if(th->cpuTotal>maxThreadTime) {

            bool merged=false;
            foreach(SolverNode *node, th->listOfSteps) {
                if(node && !node->listOfMergedNodes.isEmpty()) {
                    merged=true;
                    foreach(SolverNode *mergedNode, node->listOfMergedNodes) {
                        if(mergedNode->cpuTime > maxNodeTime) {
                            maxNodeTime=mergedNode->cpuTime;
                            nodeToMove=mergedNode;
                        }
                    }
                }
            }

            if(merged) {
                maxThreadTime = th->cpuTotal;
                biggestThreadWithMergedNode=th;
            }
        }
    }

    if(nodeToMove && nodeToMove->modelIndex.isValid()) {
        model.setData( nodeToMove->modelIndex, nodeToMove->modelIndex.data().toString().append("TOMOVE") );
    }
}

void Renderer::Clear()
{
    mutex.lock();
    stop=true;
    numberOfThreads=0;
    numberOfSteps=0;
    foreach(RenderThread *th, listOfThreads) {
        delete th;
    }
    listOfThreads.clear();
    mutex.unlock();
}

void Renderer::SetNbThreads(int nbThreads)
{
    Clear();

    mutex.lock();
    maxNumberOfThreads = nbThreads;
    if(maxNumberOfThreads<=0) maxNumberOfThreads = 4;
    InitThreads();
    stop=false;
    mutex.unlock();
}

void Renderer::OnNewRenderingOrder(orderedNodes *newSteps)
{
    mutex.lock();
    if(stop) {
        mutex.unlock();
        return;
    }

    int step = -1;
    int threadCount = -1;

    numberOfSteps = step;
    numberOfThreads = threadCount;

    model.clear();
    for(int i=0; i<maxNumberOfThreads; i++) {
        model.setHorizontalHeaderItem(i, new QStandardItem( QString::number(i+1) ) );
    }

    foreach(RenderThread *th, listOfThreads) {
        th->ResetSteps();
    }

    listOfNodesToMerge.clear();

    orderedNodes::iterator i = newSteps->begin();
    while (i != newSteps->end()) {

        step = i.key();
        if(step>numberOfSteps)
            numberOfSteps=step;

        SolverNode *node = i.value();
        node->minRenderOrderOri = node->minRenderOrder;
        node->maxRenderOrderOri = node->maxRenderOrder;

        bool found=false;

        //try to find a place in threads
        int j=0;
        while(!found && j<maxNumberOfThreads) {
            if(listOfThreads.value(j)->SetStep( node )) {
                if(j>numberOfThreads)
                    numberOfThreads=j;
                found=true;
            }
            j++;
        }

        if(!found) {
            //no place found, try again
            j=0;
            int bestThread = -1;
            int bestMod = -1;
            while(j<maxNumberOfThreads) {
                int nbMods = listOfThreads.value(j)->NeededModificationsToInsertNode( node );
                if(nbMods!=-1 && (bestMod==-1 ||  nbMods<bestMod) ) {
                    found=true;
                    bestMod = nbMods;
                    bestThread = j;
                }
                j++;
            }

            if(found) {
                listOfThreads.value(bestThread)->NeededModificationsToInsertNode( node, true );
            } else {
                listOfNodesToMerge << node;
            }
        }
        ++i;
    }

    for(int i=0; i<maxNumberOfThreads; i++) {
        listOfThreads.value(i)->AddToModel(&model,i);
    }

    //add nodes to merge
    foreach(SolverNode *node, listOfNodesToMerge) {
        int j=0;
        bool found=false;
        while(!found && j<maxNumberOfThreads) {
            if(listOfThreads.value(j)->MergeNodeInStep(node)) {
                found = true;
                model.insertRow(node->minRenderOrder+1);
//                QStandardItem *item = new QStandardItem(QString("merged[%1:%2][%3:%4]")
//                                                        .arg(node->minRenderOrder)
//                                                        .arg(node->maxRenderOrder)
//                                                        .arg(node->minRenderOrderOri)
//                                                        .arg(node->maxRenderOrderOri));
//                //add objects names to model
//                foreach( QSharedPointer<Connectables::Object> objPtr, node->listOfObj) {
//                    if(!objPtr.isNull() && !objPtr->GetSleep()) {
//                        item->setText( item->text().append("\n" + objPtr->objectName()) );
//                    }
//                }
                QStandardItem *item = new QStandardItem();
                model.setItem(node->minRenderOrder+1, j, item);
                node->modelIndex = item->index();
            }
            j++;
        }
    }

    numberOfSteps++;
    numberOfThreads++;

    QTimer::singleShot(500, this, SLOT(Optimize()));

    mutex.unlock();
}

void Renderer::StartRender()
{

    if(!mutex.tryLock()) {
        debug2(<<"Renderer::StartRender can't lock")
        return;
    }

    if(stop) {
        mutex.unlock();
        return;
    }

    if(numberOfThreads==0) {
        mutex.unlock();
        return;
    }

    sem.release(numberOfThreads);
    for(int currentStep=-1; currentStep<numberOfSteps; currentStep++) {
        sem.acquire(numberOfThreads);
        for(int i=0;i<numberOfThreads; i++) {
            RenderThread *th = listOfThreads.value(i);
            th->step = currentStep;
            th->sem.release();
        }
    }
    sem.acquire(numberOfThreads);
    mutex.unlock();
}

