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

#include "solvernode.h"
#include "renderernode2.h"

SolverNode::SolverNode() :
    Node(),
    loopFlag(0),
    countSteps(0)
{
}

SolverNode::SolverNode(const SolverNode &c) :
    Node(c),
    loopFlag(c.loopFlag),
    countSteps(c.countSteps),
    listParents(c.listParents),
    listChilds(c.listChilds)
{

}

SolverNode::SolverNode(const RendererNode2 &c) :
    Node(),
    loopFlag(0),
    countSteps(0)
{
    minRenderOrder=c.minRenderOrderOri;
    maxRenderOrder=c.maxRenderOrderOri;
    internalDelay=0;
    totalDelayAtOutput=0;
    listOfObj=c.listOfObj;
}

void SolverNode::AddChild(SolverNode *child)
{
    listChilds << child;
    child->listParents << this;
}

void SolverNode::AddParent(SolverNode *parent)
{
    listParents << parent;
    parent->listChilds << this;
}

void SolverNode::RemoveChild(SolverNode *child)
{
    listChilds.removeAll(child);
    child->listParents.removeAll(this);
}

void SolverNode::RemoveParent(SolverNode *parent)
{
    listParents.removeAll(parent);
    parent->listChilds.removeAll(this);
}

void SolverNode::ResetLoopFlags() {
    loopFlag=false;
    countSteps = 0;
}

bool SolverNode::IsRoot()
{
    if(listParents.isEmpty())
        return true;

    return false;
}

bool SolverNode::IsTail()
{
    if(listChilds.isEmpty())
        return true;

    return false;
}

bool SolverNode::DistanceFromRoot(int &steps)
{
    //already been there, continue only if this route is shorter
    if(loopFlag && steps >= countSteps) {
        return true;
    }

    countSteps = steps;
    loopFlag=true;

    if(IsRoot())
        return true;

    steps++;


    //find the best route in parents nodes
    int bestSteps=99999;
    foreach(SolverNode* parent, listParents) {
        int copySteps = steps;
        if(parent->DistanceFromRoot(copySteps)) {
            bestSteps=std::min(bestSteps, copySteps);
         }
    }
    steps=bestSteps;
    return true;
}

bool SolverNode::DistanceFromTail(int &steps)
{
    //already been there, continue only if this route is shorter
    if(loopFlag && steps >= countSteps) {
        return true;
    }

    countSteps = steps;
    loopFlag=true;

    if(IsTail())
        return true;

    steps++;

    //find the best route in parents nodes
    int bestSteps=99999;
    foreach(SolverNode* child, listChilds) {
        int copySteps = steps;
        if(child->DistanceFromTail(copySteps)) {
            bestSteps=std::min(bestSteps, copySteps);
        }
    }
    steps=bestSteps;
    return true;
}

bool SolverNode::DetectLoopback(QList<SolverNode*>&listLoop)
{
    //we're back : it's a loop
    if(!listLoop.isEmpty() && listLoop.first()==this) {
       return true;
    }

    //already solved this branch : return
    if(loopFlag)
        return false;

    loopFlag=true;
    listLoop << this;

    foreach(SolverNode* child,listChilds) {
        QList<SolverNode*>copyLoop = listLoop;
        if(child->DetectLoopback(copyLoop)) {
            listLoop = copyLoop;
            return true;
        }
    }
    return false;
}

long SolverNode::GetParentMaxDelay()
{
    //get the internal delay from parents
    long parentMaxDelay=0L;
    foreach(SolverNode *parent,listParents) {
        parentMaxDelay = std::max(parentMaxDelay,parent->totalDelayAtOutput);
    }
    return parentMaxDelay;
}

void SolverNode::UpdateInitialDelay()
{
    internalDelay=0L;
    foreach(QSharedPointer<Connectables::Object>obj, listOfObj) {
        internalDelay+=obj->GetInitDelay();
    }

    totalDelayAtOutput = internalDelay+GetParentMaxDelay();

    foreach(SolverNode *child, listChilds) {
        child->UpdateInitialDelay();
    }
}

int SolverNode::SetMinRenderOrder(int order)
{
    minRenderOrder = std::max(minRenderOrder,order);
    foreach(SolverNode *parent,listParents) {
        minRenderOrder = std::max(minRenderOrder,parent->minRenderOrder+1);
    }

    int maxstep=minRenderOrder;
    foreach(SolverNode *child, listChilds) {
        maxstep=std::max(maxstep,child->SetMinRenderOrder(minRenderOrder+1));
    }

    return maxstep;
}

void SolverNode::SetMaxRenderOrder(int order)
{
    maxRenderOrder = std::min(maxRenderOrder,order);

    foreach(SolverNode *child,listChilds) {
        maxRenderOrder = std::min(maxRenderOrder,child->maxRenderOrder-1);
    }

    foreach(SolverNode *parent, listParents) {
        parent->SetMaxRenderOrder(maxRenderOrder-1);
    }
}

void SolverNode::ReconnectChildsTo(SolverNode *newParent)
{
    //connect childs to outputline
    foreach(SolverNode *child, listChilds) {
        newParent->AddChild(child);
    }
    //and remove them
    foreach(SolverNode *child, listChilds) {
        RemoveChild(child);
    }
}

void SolverNode::ReconnectParentsTo(SolverNode *newChild)
{
    //connect childs to outputline
    foreach(SolverNode *parent, listParents) {
        newChild->AddParent(parent);
    }
    //and remove them
    foreach(SolverNode *parent, listParents) {
        RemoveParent(parent);
    }
}

bool SolverNode::MergeWithParentNode()
{
    if(listParents.count()!=1) {
        LOG("MergeWithParentNode need only one parent");
        return false;
    }

    SolverNode *parent = listParents.first();
    if(parent->listChilds.count()!=1) {
        LOG("MergeWithParentNode parent must have one child only");
        return false;
    }

    parent->listOfObj << listOfObj;
    parent->listChilds = listChilds;
    parent->maxRenderOrder = maxRenderOrder;

    foreach(SolverNode *child, listChilds) {
        child->listParents.removeAll(this);
        child->listParents << parent;
    }
    return true;
}

