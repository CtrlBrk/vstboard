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

#include "optimizerstep.h"
#include "optimizer.h"
#include "renderernode.h"

OptimizerStep::OptimizerStep(int step, int nbThreads, Optimizer *parent) :
    step(step),
    nbThreads(nbThreads),
    parent(parent),
    nextStep(0)
{
}

OptimizerStep::OptimizerStep(const OptimizerStep &s) :
    listOfNodes(s.listOfNodes),
    step(s.step),
    nbThreads(s.nbThreads),
    parent(s.parent),
    nextStep(s.nextStep)
{
    foreach(OptimizeStepThread *th, s.listOfThreads) {
        listOfThreads << new OptimizeStepThread(*th);
    }
}

OptimizerStep::~OptimizerStep()
{
    ClearThreads();
    listOfNodes.clear();
}

void OptimizerStep::ClearNodes()
{
    foreach(RendererNode *node, listOfNodes) {
        delete node;
    }
    listOfNodes.clear();
}

void OptimizerStep::ClearThreads()
{
    foreach(OptimizeStepThread* th, listOfThreads) {
        delete th;
    }
    listOfThreads.clear();
}

void OptimizerStep::CreateThreads()
{
    ClearThreads();
    for(int t=0; t<nbThreads; t++) {
        listOfThreads << new OptimizeStepThread();
    }
}

void OptimizerStep::AddNode(RendererNode *node)
{
    listOfNodes << node;
    node->minRenderOrder = node->maxRenderOrder = step;
}

void OptimizerStep::SetNbThreads(int nb)
{
    nbThreads = nb;
}

void OptimizerStep::MapOnThreadsRandomly()
{
    CreateThreads();

    int currentThread = 0;
    QList<RendererNode*>ln = listOfNodes;

    while(!ln.isEmpty()) {
        OptimizeStepThread *th = listOfThreads.value(currentThread);
        RendererNode *node = ln.takeFirst();
        th->listOfNodes << node;

        currentThread++;
        if(currentThread==nbThreads)
            currentThread=0;
    }
}

void OptimizerStep::Optimize()
{
    //sort nodes by cpu usage
    qSort(listOfNodes.begin(), listOfNodes.end(), CompareNodeSpeed);

    CreateThreads();
    QList<RendererNode*>newLstNodes = listOfNodes;

    while(!newLstNodes.isEmpty()) {

        //add biggest node to the smallest thread
        OptimizeStepThread *th = listOfThreads.first();
        RendererNode *node = newLstNodes.takeLast();

        th->listOfNodes << node;
        th->cpuTime+=node->cpuTime;

        //sort threads by cpu usage
        qSort(listOfThreads.begin(), listOfThreads.end(), CompareThreadSpeed);
    }

}

void OptimizerStep::OptimizeSpannedNodes()
{
    if(!parent)
        return;

    nextStep = parent->GetStep(step+1);
    if(!nextStep)
        return;

    //expand a spanned node
    foreach(OptimizeStepThread *th, listOfThreads) {
        RendererNode *spannedNode = th->spanFor;
        if(spannedNode && spannedNode->maxRenderOrderOri>step) {
            OptimizerStep tmpStep(*this);
            tmpStep.SetNbThreads(nbThreads-1);
            //tmpStep.listOfNodes.removeAll(spannedNode);
            tmpStep.Optimize();

            //the resulting cpu usage :
            long tmpCpuTime=tmpStep.listOfThreads.last()->cpuTime;

            if(tmpCpuTime < spannedNode->cpuTime) {
                //the spanned node is longer than the other threads, really need to be spanned

                long cpuGainForThisStep = spannedNode->cpuTime - tmpCpuTime;

                //try to span the node
                if(NextStepCanAcceptSpannedNode(spannedNode, cpuGainForThisStep)) {
                    spannedNode->maxRenderOrder = step+1;
                    ClearThreads();
                    foreach(OptimizeStepThread *th, tmpStep.listOfThreads) {
                        listOfThreads << new OptimizeStepThread(*th);
                    }
                }
            }
        }
    }

    foreach(RendererNode *node, listOfNodes) {
        if(node->maxRenderOrderOri > step) {
            //this node can be postpone or spanned

            //the spanned node need one thread, create a temporary step with one less thread and add all other nodes
            OptimizerStep tmpStep(*this);
            tmpStep.SetNbThreads(nbThreads-1);
            tmpStep.listOfNodes.removeAll(node);
            tmpStep.Optimize();

            //the resulting cpu usage :
            long tmpCpuTime=tmpStep.listOfThreads.last()->cpuTime;

            if(tmpCpuTime < node->cpuTime) {
                //the spanned node is longer than the other threads, really need to be spanned

                long cpuGainForThisStep = node->cpuTime - tmpCpuTime;

                //try to span the node
                if(NextStepCanAcceptSpannedNode(node, cpuGainForThisStep)) {
                    nbThreads--;
                    OptimizeStepThread *th = new OptimizeStepThread();
                    th->listOfNodes << node;
                    tmpStep.listOfThreads << th;
                    node->maxRenderOrder = step+1;
                    ClearThreads();
                    foreach(OptimizeStepThread *th, tmpStep.listOfThreads) {
                        listOfThreads << new OptimizeStepThread(*th);
                    }
                } else {
                    //does the next step accept this node ?
                    if(NextStepCanAcceptPostponedNode(node, cpuGainForThisStep)) {
                        listOfNodes.removeAll(node);
                        ClearThreads();
                        foreach(OptimizeStepThread *th, tmpStep.listOfThreads) {
                            listOfThreads << new OptimizeStepThread(*th);
                        }
                    }
                }
            }
        }
    }
}

bool OptimizerStep::NextStepCanAcceptSpannedNode(RendererNode *node, long cpuGainForPreviousStep)
{
    OptimizerStep *tmpStep = new OptimizerStep(*nextStep);
    tmpStep->SetNbThreads( nextStep->nbThreads-1 );
    tmpStep->Optimize();
    long newCpuTime = tmpStep->listOfThreads.last()->cpuTime;
    long oldCpuTime = nextStep->listOfThreads.last()->cpuTime;
    long cpuGain = cpuGainForPreviousStep - (newCpuTime - oldCpuTime);
    if(cpuGain>0) {
        tmpStep->SetNbThreads( tmpStep->nbThreads+1 );
        OptimizeStepThread *th = new OptimizeStepThread();
        th->spanFor = node;
        tmpStep->listOfThreads << th;
        parent->SetStep(step+1, tmpStep);
        delete nextStep;
        nextStep=tmpStep;
        return true;
    }
    delete tmpStep;
    return false;
}

bool OptimizerStep::NextStepCanAcceptPostponedNode(RendererNode *node, long cpuGainForPreviousStep)
{
    long oldCpuTime = nextStep->listOfThreads.last()->cpuTime;
    if( (node->cpuTime - oldCpuTime) > cpuGainForPreviousStep )
        return false;

    OptimizerStep *tmpStep = new OptimizerStep(*nextStep);
    tmpStep->listOfNodes << node;
    tmpStep->Optimize();
    long newCpuTime = tmpStep->listOfThreads.last()->cpuTime;
    long cpuGain = cpuGainForPreviousStep - (newCpuTime - oldCpuTime);
    if(cpuGain>0) {
        parent->SetStep(step+1, tmpStep);
        delete nextStep;
        nextStep=tmpStep;
        node->minRenderOrder = node->maxRenderOrder = nextStep->step;
        return true;
    }
    delete tmpStep;
    return false;
}

bool OptimizerStep::GetMergedNode(int thread, RendererNode **node)
{
    if(listOfThreads.size()<=thread) {
        return false;
    }

    OptimizeStepThread *th = listOfThreads.at(thread);
    return th->GetMergedNode( node );
}

QList<RendererNode*> OptimizerStep::GetListOfNodes()
{
   QList<RendererNode*>newLst;
   foreach(RendererNode *node, listOfNodes) {
       newLst << new RendererNode(*node);
   }
   return newLst;
}

bool OptimizerStep::CompareNodeSpeed(RendererNode *n1, RendererNode *n2)
{
    return n1->cpuTime < n2->cpuTime;
}

bool OptimizerStep::CompareThreadSpeed(OptimizeStepThread *t1, OptimizeStepThread *t2)
{
    return t1->cpuTime < t2->cpuTime;
}

//void OptimizerStep::AddToModel( QStandardItemModel *model )
//{
//    int col=0;
//    int row=model->rowCount();
//    foreach( OptimizeStepThread *th, listOfThreads ) {
//        th->AddToModel(model, row, col);
//        col++;
//    }
//}

//void OptimizerStep::UpdateView( QStandardItemModel *model )
//{
//    foreach( OptimizeStepThread *th, listOfThreads ) {
//        th->UpdateView(model);
//    }
//}
