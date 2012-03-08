/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#include "updatedelays.h"
#include "mainhost.h"

UpdateDelays::UpdateDelays(const hashObjects &listObjects, hashCables *listCables, const QList<SolverNode*> *listNodes) :
    listObjects(listObjects),
    listCables(listCables),
    listNodes(listNodes)
{
    ResetDelays();
    int cpt=0;
    while(AddDelays() && cpt<100) { ++cpt; }
    cpt=0;

    do {
        UpdateGlobalDelay();
        ++cpt;
    } while(SynchronizeAudioOutputs() && cpt<100);
}

/*!
  Remove delays
  */
void UpdateDelays::ResetDelays()
{
    hashCables::iterator i = listCables->begin();
    while (i != listCables->end()) {
        i.value()->SetDelay(0);
        ++i;
    }

    foreach(SolverNode *node, *listNodes) {
        if(!node->IsRoot())
            continue;
        node->UpdateInitialDelay();
    }
}

/*!
  Add delays at the end of nodes to keep in sync
  */
bool UpdateDelays::AddDelays()
{
    foreach(SolverNode *node, *listNodes) {
        long delay = node->GetParentMaxDelay();
        if(SynchronizeParentNodes(node,delay))
            return true;
    }
    return false;
}

bool UpdateDelays::SynchronizeParentNodes(SolverNode *node, long targetDelay)
{
    foreach(SolverNode *parent, node->listParents) {
        long delayToAdd = targetDelay - parent->totalDelayAtOutput;

        if(delayToAdd>0) {
//            LOG("add delay"<<delayToAdd<<parent->listOfObj.first().toStrongRef()->objectName()<<node->listOfObj.first().toStrongRef()->objectName());
            QSharedPointer<Connectables::Object>obj = parent->listOfObj.last().toStrongRef();

            QList<QSharedPointer<Connectables::Cable> >lstCables;
            GetListCablesConnectedTo(obj->GetIndex(), lstCables);
            bool mod=false;
            foreach(QSharedPointer<Connectables::Cable>cab, lstCables) {
                QSharedPointer<Connectables::Object>destObj = listObjects.value( cab->GetInfoIn().objId ); // myHost->objFactory->GetObjectFromId(cab->GetInfoIn().objId);
                if(node->listOfObj.contains(destObj)) {
                    if(cab->SetDelay(delayToAdd))
                        mod=true;
                }
            }
            parent->internalDelay+=delayToAdd;
            parent->totalDelayAtOutput+=delayToAdd;
            if(mod)
                return true;
        }
    }
    return false;
}

void UpdateDelays::UpdateGlobalDelay()
{
    //get the maximum delay at audio out
    globalDelay=0L;
    foreach(SolverNode *node, *listNodes) {
        if(node->totalDelayAtOutput>globalDelay) {
            //only for nodes containing audio output
            foreach(QSharedPointer<Connectables::Object>obj, node->listOfObj) {
                if(obj->info().objType==ObjType::AudioInterfaceOut)
                    globalDelay=node->totalDelayAtOutput;
            }
        }
    }
}

bool UpdateDelays::SynchronizeAudioOutputs()
{
    //set an equal delay on all outputs
    foreach(SolverNode *node, *listNodes) {
        bool isOutput=false;
        foreach(QSharedPointer<Connectables::Object>obj, node->listOfObj) {
            if(obj->info().objType==ObjType::AudioInterfaceOut) {
                    isOutput=true;
            }
        }
        if(isOutput) {
            if(SynchronizeParentNodes(node,globalDelay)) {
                return true;
            }
        }
    }

    return false;
}

void UpdateDelays::GetListCablesConnectedTo(quint16 objId, QList<QSharedPointer<Connectables::Cable> > &list)
{
    hashCables::const_iterator i = listCables->constBegin();
    while (i != listCables->constEnd()) {
        if(i.key().objId == objId)
            list << i.value();
        ++i;
    }
}

///*!
//  Insert a delay node between node and chilnode
//  */
//void PathSolver::CreateDelayNode(SolverNode *node, SolverNode *childNode, long delay)
//{
//    QSharedPointer<Connectables::Object>outObj = node->listOfObj.last();
//    QSharedPointer<Connectables::Object>inObj = childNode->listOfObj.first();
//    QSharedPointer<Connectables::Container>container = myHost->objFactory->GetObjectFromId(outObj->GetContainerId()).staticCast<Connectables::Container>();


//    node->RemoveChild(childNode);
//    childNode->totalDelayAtOutput+=delay;
////    LOG("add obj"<<delayObj->objectName());

//    qreal objPosY=.0f;

//    //find affected cables
//    foreach(Connectables::Pin* outPin, outObj->GetListAudioPinOut()->listPins) {
//        ConnectionInfo outInfo = outPin->GetConnectionInfo();
//        QList<ConnectionInfo>lstIn;
//        GetListPinsConnectedTo(outInfo, lstIn);
//        foreach(ConnectionInfo inInfo, lstIn) {
//            if(inObj->GetIndex() == inInfo.objId) {
////                //found a cable !
////                LOG("add delay"<<delay<<outObj->objectName()<<inObj->objectName());
//                SolverNode *newNode = new SolverNode();
//                node->AddChild(newNode);
//                newNode->AddChild(childNode);
//                listNodes<<newNode;
//                ObjectInfo info;
//                info.nodeType = NodeType::object;
//                info.objType = ObjType::Buffer;
//                info.initDelay = delay;
//                QSharedPointer<Connectables::Buffer>delayObj = myHost->objFactory->NewObject(info).staticCast<Connectables::Buffer>();
//                newNode->listOfObj << delayObj;
//                container->UserAddObject(delayObj);
//                delayObj->SetSolverNode(node);
//                container->UserAddCable(outInfo, delayObj->GetListAudioPinIn()->GetPin(0)->GetConnectionInfo());
//                container->UserAddCable(delayObj->GetListAudioPinOut()->GetPin(0)->GetConnectionInfo(), inInfo);
//                container->UserRemoveCable(outInfo, inInfo);

//                ObjectContainerAttribs attr;
//                inObj->GetContainerAttribs(attr);
//                attr.position.rx()-=130;
//                attr.position.ry()+=objPosY;
//                objPosY+=50;
//                delayObj->SetContainerAttribs(attr);
//            }
//        }
//    }

//    //can be a bridge too
//    foreach(Connectables::Pin* outPin, outObj->GetListBridgePinOut()->listPins) {
//        ConnectionInfo outInfo = outPin->GetConnectionInfo();
//        QList<ConnectionInfo>lstIn;
//        GetListPinsConnectedTo(outInfo, lstIn);
//        foreach(ConnectionInfo inInfo, lstIn) {
//            if(inObj->GetIndex() == inInfo.objId) {
////                //found a cable !
////                LOG("add delay"<<delay<<outObj->objectName()<<inObj->objectName());
//                SolverNode *newNode = new SolverNode();
//                node->AddChild(newNode);
//                newNode->AddChild(childNode);
//                listNodes<<newNode;
//                ObjectInfo info;
//                info.nodeType = NodeType::object;
//                info.objType = ObjType::Buffer;
//                info.initDelay = delay;
//                QSharedPointer<Connectables::Buffer>delayObj = myHost->objFactory->NewObject(info).staticCast<Connectables::Buffer>();
//                newNode->listOfObj << delayObj;
//                container->UserAddObject(delayObj);
//                delayObj->SetSolverNode(node);
//                container->UserAddCable(outInfo, delayObj->GetListAudioPinIn()->GetPin(0)->GetConnectionInfo());
//                container->UserAddCable(delayObj->GetListAudioPinOut()->GetPin(0)->GetConnectionInfo(), inInfo);
//                container->UserRemoveCable(outInfo, inInfo);

//                ObjectContainerAttribs attr;
//                inObj->GetContainerAttribs(attr);
//                attr.position.rx()-=130;
//                attr.position.ry()+=objPosY;
//                objPosY+=50;
//                delayObj->SetContainerAttribs(attr);
//            }
//        }
//    }
//}
