/**************************************************************************
#    Copyright 2010-2020 Raphaël François
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

#include "mainhost.h"
#include "pathsolver.h"
#include "globals.h"
#include "projectfile/projectfile.h"
#include "connectables/objectfactory.h"
#include "connectables/buffer.h"
#include "updatedelays.h"

PathSolver::PathSolver(QObject *parent) :
    QObject(parent)
{

}

PathSolver::~PathSolver()
{
    Clear();
}

void PathSolver::Clear()
{
    mutex.lock();
    foreach(SolverNode* line, listNodes) {
        delete line;
    }
    listNodes.clear();
    listAciveObjects.clear();
    mutex.unlock();

}

long PathSolver::GetNodes(const hashObjects &lstObj, const hashCables &cables, QList<SolverNode*> &nodes)
{
    listObjects = lstObj;
    listCables = cables;

    CreateNodes();
    PutParentsInNodes();
    int cpt=0;
    while(ChainNodes() && cpt<100) { ++cpt; }
    UnwrapLoops();
    UpdateDelays dly(listObjects,&listCables,&listNodes);
//    while(ChainNodes() && cpt<100) { ++cpt; }
    RemoveUnusedNodes();
    SetMinAndMaxStep();
    nodes = listNodes;
    listNodes.clear();
    return dly.GetDelay();
}

//void PathSolver::GetListPinsConnectedTo(ConnectionInfo out, QList<ConnectionInfo> &list)
//{
//    hashCables::const_iterator i = listCables.constFind(out);
//    while (i != listCables.constEnd()  && i.key() == out) {
//        list << i.value()->GetInfoIn();
//        ++i;
//    }
//}

/*!
  Create nodes for each objects
  */
void PathSolver::CreateNodes()
{
//    const Connectables::hashObjects listObjects = myHost->objFactory->GetListObjects();
    hashObjects::const_iterator i = listObjects.constBegin();
    while(i!=listObjects.constEnd()) {
        QSharedPointer<Connectables::Object> objPtr = i.value();

        //don't add parked objects
        if(objPtr && objPtr->parkingId==FixedObjId::ND) {
//            if(objPtr->info().nodeType!=NodeType::bridge && objPtr->info().nodeType!=NodeType::container) {
            if( objPtr->info().nodeType!=NodeType::container) {
                SolverNode *node = new SolverNode();
                listNodes << node;
                node->listOfObj << objPtr;
                objPtr->SetSolverNode(node);
            }
        }
        ++i;
    }
}

/*!
  List all childs and parents in each node
  */
void PathSolver::PutParentsInNodes()
{
    foreach(SolverNode *node,listNodes) {
        foreach(QSharedPointer<Connectables::Object> parent,GetListParents(node->listOfObj.first())) {
            if(parent)// && parent->info().nodeType!=NodeType::bridge)
                node->AddParent( parent->GetSolverNode() );
        }
//        foreach(QSharedPointer<Connectables::Object> child,GetListChildren(node->listOfObj.last())) {
//            if(child)// && child->info().nodeType!=NodeType::bridge)
//                node->listChilds << child->GetSolverNode();
//        }
    }

//    foreach(SolverNode *node,listNodes) {
//        //no parents, no childs : don't render it
//        if(node->listChilds.isEmpty() && node->listParents.isEmpty()) {
//            listNodes.removeAll(node);
//            delete node;
//        }
//    }
}

/*!
    Group nodes with only one parent or child
    \return true if a node was grouped
    */
bool PathSolver::ChainNodes()
{
    foreach(SolverNode* node, listNodes) {
        if(node->listChilds.count() == 1) {
            SolverNode *childNode = node->listChilds.first();
            if( childNode->listParents.count() == 1) {
                if(childNode->MergeWithParentNode()) {
                    listNodes.removeAll(childNode);
                    delete childNode;
                    return true;
                }
            }
        }
    }

    return false;
}

void PathSolver::RemoveUnusedNodes()
{
    foreach(SolverNode *node, listNodes) {
        if(node->BridgesOnly()) {
            foreach(SolverNode *child, node->listChilds) {
                child->listParents.removeAll(node);
                child->listParents << node->listParents;
            }
            foreach(SolverNode *parent, node->listParents) {
                parent->listChilds.removeAll(node);
                parent->listChilds << node->listChilds;
            }
            listNodes.removeAll(node);
            delete node;
        }
    }
}

void PathSolver::UnwrapLoops()
{
    //unwrap loops
    foreach(SolverNode *node,listNodes) {
        foreach(SolverNode *n,listNodes) {
            n->ResetLoopFlags();
        }

        //find loop
        QList<SolverNode*>loop;
        if(node->DetectLoopback(loop)) {

            //find the node close to a root
            QList<SolverNode*>listStarts = ListOfGoodStarts(loop);

            SolverNode* bestStart=0;

            //we have a winner
            if(listStarts.size()==1)
                bestStart = listStarts.first();

            //it's a draw... try to find the best one
            if(listStarts.size()>1)
                listStarts = BestStartsInAList(loop,listStarts);

            //now we have a winner
            if(listStarts.size()==1)
                bestStart = listStarts.first();

            //a draw, again. i don't know what else to check : take the first in the list
            if(listStarts.size()>1)
                bestStart = listStarts.first();

            if(bestStart) {
                //opened the loop : no more feedback
                QList<SolverNode*>openedLoop = OpenLoop(loop, bestStart);

                //create a copy of the loop
                QList<SolverNode*>copiedLoop = CopyNodesChain(openedLoop);

                //insert the copied loop before the original one
                openedLoop.first()->ReconnectParentsTo(copiedLoop.first());
                copiedLoop.last()->AddChild(openedLoop.first());

            } else {
                Q_ASSERT(false);
                //bestEnd not found : wtf ?
            }
        }
    }
}

/*!
  Find the rendering window, minimum and maximum rendering step
  */
void PathSolver::SetMinAndMaxStep()
{
    int maxStep=0;
    foreach(SolverNode *node, listNodes) {
        if(!node->IsRoot())
            continue;
        maxStep = std::max(maxStep, node->SetMinRenderOrder( 0 ));
    }

    foreach(SolverNode *node, listNodes) {
        if(!node->IsTail())
            continue;
        node->SetMaxRenderOrder(maxStep);
    }
}

//return a list of good starts by looking at the nodes close to a root
QList<SolverNode*> PathSolver::ListOfGoodStarts(const QList<SolverNode*>&loop)
{
    QList<SolverNode*> bestStarts;
    int minStep=99999;

    foreach(SolverNode *node, loop) {
        foreach(SolverNode *n,listNodes) {
            n->ResetLoopFlags();
        }
        int steps=0;

        if(node->DistanceFromRoot(steps)) {
            //detect equality
            if(steps==minStep) {
                bestStarts << node;
            }
            //this is a new best
            if(steps<minStep) {
                bestStarts.clear();
                minStep=steps;
                bestStarts << node;
            }
        }
    }

    return bestStarts;
}

//return the best start of a loop by finding the ending node close to a tail
QList<SolverNode*> PathSolver::BestStartsInAList(const QList<SolverNode*>&loop, const QList<SolverNode*>&possibleStarts)
{
    QList<SolverNode*> bestStarts;
    int minStep=99999;

    foreach(SolverNode *triedStart, possibleStarts) {
        //which node would be the end one in this case
        SolverNode *triedEnd;
        if(triedStart == loop.first())
            triedEnd = loop.last();
        else {
            triedEnd = loop.at( loop.indexOf(triedStart)-1 );
        }

        foreach(SolverNode *n,listNodes) {
            n->ResetLoopFlags();
        }
        int steps=0;

        if(triedEnd->DistanceFromTail(steps)) {
            if(steps==minStep) {
                bestStarts << triedStart;
            }
            if(steps<minStep) {
                bestStarts.clear();
                bestStarts << triedStart;
                minStep=steps;
            }
        }
    }

    return bestStarts;
}

//copy a chain of nodes
QList<SolverNode*> PathSolver::CopyNodesChain(const QList<SolverNode*>&chain)
{
    QList<SolverNode*>copiedChain;
    SolverNode *parentNode=0;
    foreach(SolverNode *node, chain) {
        SolverNode *copiedNode = new SolverNode();
        listNodes << copiedNode;
        copiedNode->listOfObj = node->listOfObj;

        if(parentNode)
            parentNode->AddChild(copiedNode);

        parentNode = copiedNode;
        copiedChain << copiedNode;
    }
    return copiedChain;
}

//cut a loop before the selected start
QList<SolverNode*> PathSolver::OpenLoop(const QList<SolverNode*>&loop, SolverNode* startingNode)
{
    QList<SolverNode*>destLoop;

    //rotate the loop, it must start with startingNode
    int pos=loop.indexOf(startingNode);
    for(int i=0;i<loop.size();i++) {
        if(pos==loop.size())
            pos=0;
        destLoop << loop.at(pos);
        ++pos;
    }

    //remove connections between last and first to break the feedback
    destLoop.first()->RemoveParent(destLoop.last());

    return destLoop;
}


//get the objects connected to the input pins
QList< QSharedPointer<Connectables::Object> >PathSolver::GetListParents( QSharedPointer<Connectables::Object> objPtr)
{
    QList< QSharedPointer<Connectables::Object> >listParents;

    hashCables::iterator i = listCables.begin();
    while (i != listCables.end()) {
        QSharedPointer<Connectables::Object> parentPtr = listObjects.value(i.key().objId); //myHost->objFactory->GetObjectFromId(i.key().objId);
        if(parentPtr) {
            if(i.value()->GetInfoIn().objId == objPtr->GetIndex()) {
                if(!listParents.contains(parentPtr)) {
                    listParents << parentPtr;
//                    if(parentPtr->info().nodeType == NodeType::bridge)
//                        GetListParentsOfBridgePin(i.key(),listParents);
                }
            }
        }
        ++i;
    }

    return listParents;
}

//void PathSolver::GetListParentsOfBridgePin(const ConnectionInfo &info, QList< QSharedPointer<Connectables::Object> > &listParents)
//{
//    hashCables::iterator i = listCables.begin();
//    while (i != listCables.end()) {
//        QSharedPointer<Connectables::Object> parentPtr = myHost->objFactory->GetObjectFromId(i.key().objId);
//        if(parentPtr) {
//            if(i.value().objId == info.objId && i.value().pinNumber == info.pinNumber && i.value().type == info.type) {
//                if(!listParents.contains(parentPtr)) {
//                    listParents << parentPtr;
//                    if(parentPtr->info().nodeType == NodeType::bridge)
//                        GetListParentsOfBridgePin(i.key(),listParents);
//                }
//            }
//        }
//        ++i;
//    }
//}

//get the objects connected to the output pins
QList< QSharedPointer<Connectables::Object> >PathSolver::GetListChildren( QSharedPointer<Connectables::Object> objPtr)
{
    QList< QSharedPointer<Connectables::Object> >listChildren;

    hashCables::iterator i = listCables.begin();
    while (i != listCables.end()) {
        QSharedPointer<Connectables::Object> childPtr = listObjects.value( i.value()->GetInfoIn().objId ); //myHost->objFactory->GetObjectFromId(i.value()->GetInfoIn().objId);
        if(childPtr) {
            if(i.key().objId == objPtr->GetIndex()) {
                if(!listChildren.contains(childPtr)) {
                    listChildren << childPtr;
//                    if(childPtr->info().nodeType == NodeType::bridge)
//                        GetListChildrenOfBridgePin(i.value(),listChildren);
                }
            }
        }
        ++i;
    }

    return listChildren;
}

//void PathSolver::GetListChildrenOfBridgePin(const ConnectionInfo &info, QList< QSharedPointer<Connectables::Object> > &listChildren)
//{
//    hashCables::iterator i = listCables.begin();
//    while (i != listCables.end()) {
//        QSharedPointer<Connectables::Object> childPtr = myHost->objFactory->GetObjectFromId(i.value().objId);
//        if(childPtr) {
//            if(i.key().objId == info.objId && i.key().pinNumber == info.pinNumber && i.key().type == info.type) {
//                if(!listChildren.contains(childPtr)) {
//                    listChildren << childPtr;
//                    if(childPtr->info().nodeType == NodeType::bridge) {
//                        GetListChildrenOfBridgePin(i.value(),listChildren);
//                    }
//                }
//            }
//        }
//        ++i;
//    }
//}

