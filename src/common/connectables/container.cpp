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
#include "container.h"
#include "objectfactory.h"
#include "mainhost.h"
#include "projectfile/projectfile.h"
#include "commands/comaddobject.h"
#include "commands/comremoveobject.h"
#include "commands/comaddcable.h"
#include "commands/comdisconnectpin.h"

using namespace Connectables;

/*!
  \class Connectables::Container
  \brief contains Objects an Cables, uses Bridges to communicate with other Containers
  */

/*!
  \param myHost pointer to the MainHost
  \param index object number
  \param info object description
  */
Container::Container(MainHost *myHost,int index, const ObjectInfo &info) :
    Object(myHost,index, info ),
    bridgeIn(0),
    bridgeOut(0),
    bridgeSend(0),
    bridgeReturn(0),
    containersParkingId(FixedObjId::ND),
    optimizerFlag(false),
    currentContainerProgram(0),
    cablesNode(QModelIndex()),
    progToSet(-1),
    loadingMode(false)
{
//    qDeleteAll(pinLists);
//    pinLists.clear();
//    listAudioPinIn=0;
//    listAudioPinOut=0;
//    listParameterPinIn=0;
//    listParameterPinOut=0;
//    listMidiPinIn=0;
//    listMidiPinOut=0;
//    listBridgePinIn=0;
//    listBridgePinOut=0;


//    parkModel.setObjectName("parkModel"%objectName());
    LoadProgram(TEMP_PROGRAM);
//    connect(myHost,SIGNAL(BufferSizeChanged(ulong)),
//            this,SLOT(SetBufferSize(ulong)));
//    connect(myHost,SIGNAL(SampleRateChanged(float)),
//            this,SLOT(SetSampleRate(float)));
}

/*!
  Close on destruction
  */
Container::~Container()
{
    Close();
}

/*!
  Get the cables model node (cables are in a child node)
  \return the node or an invalid index
  */
//const QModelIndex &Container::GetCablesIndex()
//{
//    // the node exists, return it
//    if(cablesNode.isValid())
//        return cablesNode;

//    // the container is parked or the node is not valid, return an invalid index
//    if(parked || !modelIndex.isValid()) {
//        cablesNode = QModelIndex();
//        return cablesNode;
//    }

//    //create the cables node
//    QStandardItem *item = myHost->GetModel()->itemFromIndex(modelIndex);
//    if(!item) {
//        LOG("modelindex not found");
//        return cablesNode;
//    }
//    QStandardItem *cab = new QStandardItem("cables");
//    item->appendRow(cab);
//    cablesNode = cab->index();
//    return cablesNode;
//}

void Container::SetContainerId(qint32 id)
{
    Object::SetContainerId(id);

    foreach(QSharedPointer<Object>obj, listLoadedObjects) {
        if(obj)
            obj->SetContainerId(GetIndex());
    }
}

void Container::ConnectObjects(QVariant from, QVariant to)
{
    Object *objFrom  = from.value<Object*>();
    Object *objTo  = to.value<Object*>();

    if(!objFrom || !objTo)
        return;

    QSharedPointer<Object>pFrom = myHost->objFactory->GetObjectFromId(objFrom->GetIndex());
    QSharedPointer<Object>pTo = myHost->objFactory->GetObjectFromId(objTo->GetIndex());

    QList< QSharedPointer<Cable> >addedCables;
    QList<int>removedCables;
    currentContainerProgram->CollectCableUpdatesIds( &addedCables, &removedCables );

    ConnectObjects(pFrom,pTo,false);

//    ConnectObjects(
//        myHost->objFactory->GetObjectFromId(from),
//        myHost->objFactory->GetObjectFromId(to),
//        false
//    );

    currentContainerProgram->CollectCableUpdates();
    currentContainerProgram->CollectCableUpdatesIds();

    if(MsgEnabled()) {
        foreach(int id, removedCables) {
            MSGOBJ();
            msg.prop[MsgObject::Remove]=id;
            msgCtrl->SendMsg(msg);
        }


        foreach(QSharedPointer<Cable> cab, addedCables) {
            MSGOBJ();
            cab->GetInfos(msg);
            msgCtrl->SendMsg(msg);
        }
    }

    myHost->SetSolverUpdateNeeded();
    UpdateModificationTime();
}

/*!
  Connect the output pins of fromObjOutputs to the inputs of toObjInputs
  \param fromObjOutputs
  \param toObjInputs
  \param hidden true to create hidden cables
  */
void Container::ConnectObjects(QSharedPointer<Object> fromObjOutputs, QSharedPointer<Object> toObjInputs, bool hiddenCables)
{
    if(!fromObjOutputs || !toObjInputs)
        return;

    PinsList *a = fromObjOutputs->GetListAudioPinOut();
    if(a) a->ConnectAllTo(this,toObjInputs->GetListAudioPinIn(), hiddenCables);

    PinsList *m = fromObjOutputs->GetListMidiPinOut();
    if(m) m->ConnectAllTo(this,toObjInputs->GetListMidiPinIn(), hiddenCables);

    PinsList *b = fromObjOutputs->GetListBridgePinOut();
    if(b) b->ConnectAllTo(this,toObjInputs->GetListBridgePinIn(), hiddenCables);
}

bool Container::Close()
{
    if(closed)
        return false;

    foreach(QSharedPointer<Object> objPtr, listStaticObjects) {
        if(!objPtr)
            continue;
        ParkObject(objPtr);
    }
    listStaticObjects.clear();

    if(currentContainerProgram) {
        currentContainerProgram->Unload();
        currentContainerProgram->ParkAllObj();
        delete currentContainerProgram;
        currentContainerProgram=0;
    }

    foreach(ContainerProgram *prog, listContainerPrograms) {
        delete prog;
    }
    listContainerPrograms.clear();

    bridgeIn.clear();
    bridgeOut.clear();
    bridgeSend.clear();
    bridgeReturn.clear();

    return true;
}

void Container::SetSleep(bool sleeping)
{
    Object::SetSleep(sleeping);

    if(!currentContainerProgram)
        return;

    foreach(QSharedPointer<Object> objPtr, currentContainerProgram->listObjects) {
        if(objPtr)
            objPtr->SetSleep(GetSleep());
    }
}

//void Container::Hide()
//{
//    cablesNode=QModelIndex();

//    if(currentContainerProgram) {
//        foreach(QSharedPointer<Object> objPtr, currentContainerProgram->listObjects) {
//            if(objPtr)
//                objPtr->Hide();
//        }
//    }
//    foreach(QSharedPointer<Object> objPtr, listStaticObjects) {
//        if(objPtr)
//            objPtr->Hide();
//    }

//    Object::Hide();

//}

/*!
  Will change program on the next render loop
  \param prg a program model index
  */
void Container::SetProgram(quint32 progId)
{
    progToSet=progId;
    if(progToSet == currentProgId) {
        progToSet=-1;
        return;
    }

}

void Container::NewRenderLoop()
{

}

void Container::PostRender()
{
    if(progToSet!=-1) {
        int p =progToSet;
        progToSet=-1;
        LoadProgram(p);
    }
}

void Container::SetBufferSize(qint32 size)
{
    foreach(QSharedPointer<Object>obj, listLoadedObjects) {
        if(obj)
            obj->SetBufferSize(size);
    }
}

void Container::SetSampleRate(float rate)
{
    foreach(QSharedPointer<Object>obj, listLoadedObjects) {
        if(obj)
            obj->SetSampleRate(rate);
    }
}

void Container::LoadProgram(int prog)
{

//    QMutexLocker ml(&progLoadMutex);

    //if prog is already loaded, update model
    if(prog==currentProgId && currentContainerProgram) {
        return;
    }

    bool msgWasEnabled=MsgEnabled();
    SetMsgEnabled(false);

    SetLoadingMode(true);

    if(!listContainerPrograms.contains(prog))
        listContainerPrograms.insert(prog,new ContainerProgram(myHost,this));

    ContainerProgram *oldProg = currentContainerProgram;
    ContainerProgram *newProg = listContainerPrograms.value(prog);


    if(oldProg) {
        //update the saved rendering map
        if(optimizerFlag && currentProgId!=EMPTY_PROGRAM && currentProgId!=TEMP_PROGRAM) {
            ContainerProgram *p = listContainerPrograms.value(currentProgId,0);
            if(p)
                p->SaveRendererState();
        }

        //remove objects from the old program if not needed anymore
        foreach(QSharedPointer<Object>objPtr, oldProg->listObjects) {
            if(!newProg->listObjects.contains(objPtr)) {
                ParkChildObject(objPtr);
//                QTimer::singleShot(0, objPtr.data(), SLOT(OnHideEditor()));
                objPtr->OnHideEditor();
            }
        }

        //unload old prog
        if(currentProgId!=EMPTY_PROGRAM ) {
            oldProg->Unload();
        } else {
            oldProg = 0;
        }
    }

    currentProgId=prog;
    currentContainerProgram = new ContainerProgram(*newProg);

    //add new objects
    foreach(QSharedPointer<Object>objPtr, newProg->listObjects) {
        if(!oldProg || !oldProg->listObjects.contains(objPtr)) {
            AddChildObject(objPtr);
        }
    }

    currentContainerProgram->Load(prog);

    SetLoadingMode(false);

    if(optimizerFlag)
        currentContainerProgram->LoadRendererState();

    if(oldProg) {
        delete oldProg;
    }

    if(msgWasEnabled) {
        SetMsgEnabled(true);
        UpdateView();
    }
}

const QTime Container::GetLastModificationTime() {
    QTime parentTime;
    QTime myTime;

    if(parentContainer)
        parentTime = parentContainer.toStrongRef()->GetLastModificationTime();

    if(currentContainerProgram)
        myTime = currentContainerProgram->lastModificationTime;

    if(myTime>parentTime)
        return myTime;
    else
        return parentTime;
}

void Container::SaveProgram()
{
	//TODO : check for null pointer and temp prog ?
	if(!currentContainerProgram && currentProgId==TEMP_PROGRAM)
        return;
	if (!currentContainerProgram)
		return;

    currentContainerProgram->Save();

    delete listContainerPrograms.take(currentProgId);
    listContainerPrograms.insert(currentProgId,currentContainerProgram);
	ContainerProgram *tmp = new ContainerProgram(*currentContainerProgram);
	currentContainerProgram = tmp;
}

void Container::UnloadProgram()
{
    if(!currentContainerProgram)
        return;

    currentContainerProgram->Unload();
    delete currentContainerProgram;
    currentContainerProgram=0;

    currentProgId=EMPTY_PROGRAM;
}

/*!
  Try to remove the program now, retry later if we try to remove the current program
  */
void Container::RemoveProgram(quint32 progId)
{
    if(progId>0)
        listProgToRemove << progId;

    QList<int>remainingProgs;

    while(!listProgToRemove.isEmpty()) {
        int p = listProgToRemove.takeFirst();

        if(listContainerPrograms.contains(p)) {
            if(p == currentProgId) {
                remainingProgs << p;
                if(progToSet==-1) {
                    LOG("removing current program and no scheduled progChange "<<p<<objectName());
                }
            } else {
                delete listContainerPrograms.take(p);
            }
        } /*else {
            //the program does not exist, nothing to do
            LOG("unknown prog"<<p<<objectName()<<listContainerPrograms.keys());
        }*/
    }

    //some programs where not removed, retry later
    if(!remainingProgs.isEmpty()) {
        listProgToRemove = remainingProgs;
        QTimer::singleShot(10, this, SLOT(RemoveProgram()));
    }
}

/*!
  Add a new object in the current program
  \param objPtr shared pointer to the object
  */
void Container::AddObject(QSharedPointer<Object> objPtr)
{
    objPtr->SetContainerId(GetIndex());
//    objPtr->UnloadProgram();

    //bridges are not stored in program
    if(objPtr->info().nodeType == NodeType::bridge) {
        if(objPtr->info().objType==ObjType::BridgeIn) {
            bridgeIn=objPtr;
        }
        if(objPtr->info().objType==ObjType::BridgeOut) {
            bridgeOut=objPtr;
        }

        if(objPtr->info().objType==ObjType::BridgeSend) {
            bridgeSend=objPtr;
        }
        if(objPtr->info().objType==ObjType::BridgeReturn) {
            bridgeReturn=objPtr;
        }

        objPtr->listenProgramChanges=false;
        listStaticObjects << objPtr;
        AddChildObject(objPtr);
        objPtr->LoadProgram(currentProgId);
        return;
    }

    if(!listLoadedObjects.contains(objPtr))
        listLoadedObjects << objPtr;
    currentContainerProgram->AddObject(objPtr);
    objPtr->LoadProgram(currentProgId);
}

/*!
  Place an object and park it
  \param objPtr shared pointer to the object
  */
void Container::AddParkedObject(QSharedPointer<Object> objPtr)
{
    objPtr->SetContainerId(GetIndex());
    objPtr->UnloadProgram();

    listLoadedObjects << objPtr;
    ParkChildObject(objPtr);
}

void Container::UserAddObject(const QSharedPointer<Object> &objPtr,
                              InsertionType::Enum insertType,
                              QList< QPair<ConnectionInfo,ConnectionInfo> > *listOfAddedCables,
                              QList< QPair<ConnectionInfo,ConnectionInfo> > *listOfRemovedCables,
                              const QSharedPointer<Object> &targetPtr)
{
    AddObject(objPtr);
    objPtr->SetMsgEnabled(MsgEnabled());

    if(MsgEnabled())
        objPtr->UpdateViewNow();

    if(targetPtr) {
        currentContainerProgram->CollectCableUpdates( listOfAddedCables, listOfRemovedCables );
        QList< QSharedPointer<Cable> >addedCables;
        QList<int>removedCables;
        currentContainerProgram->CollectCableUpdatesIds( &addedCables, &removedCables );


        switch(insertType) {
            case InsertionType::InsertBefore:
                MoveInputCablesFromObj(objPtr, targetPtr);
                ConnectObjects(objPtr, targetPtr, false);
                break;
            case InsertionType::InsertAfter:
                MoveOutputCablesFromObj(objPtr, targetPtr);
                ConnectObjects(targetPtr, objPtr, false);
                break;
            case InsertionType::AddBefore:
                ConnectObjects(objPtr, targetPtr, false);
                break;
            case InsertionType::AddAfter:
                ConnectObjects(targetPtr, objPtr, false);
                break;
            case InsertionType::Replace: {
                CopyCablesFromObj(objPtr, targetPtr);
                (targetPtr)->CopyStatusTo(objPtr);
                ParkObject(targetPtr);

                if(MsgEnabled()) {
//                    MSGOBJ();
                    MSGOBJ();
                    msg.prop[MsgObject::Remove]=targetPtr->GetIndex();
                    msgCtrl->SendMsg(msg);
                }

                break;
            }
            case InsertionType::NoInsertion:
                break;
        }
        currentContainerProgram->CollectCableUpdates();
        currentContainerProgram->CollectCableUpdatesIds();

        if(MsgEnabled()) {
            foreach(int id, removedCables) {
//                MSGOBJ();
                MSGOBJ();
                msg.prop[MsgObject::Remove]=id;
                msgCtrl->SendMsg(msg);
            }


            foreach(QSharedPointer<Cable> cab, addedCables) {
//                MSGOBJ();
                MSGOBJ();
                cab->GetInfos(msg);
                msgCtrl->SendMsg(msg);
            }
        }

    }
    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }

}


void Container::UserParkObject(QSharedPointer<Object> objPtr,
                               RemoveType::Enum removeType,
                               QList< QPair<ConnectionInfo,ConnectionInfo> > *listOfAddedCables,
                               QList< QPair<ConnectionInfo,ConnectionInfo> > *listOfRemovedCables)
{

    MSGOBJ();
    msg.prop[MsgObject::Remove]=objPtr->GetIndex();
    msgCtrl->SendMsg(msg);

    currentContainerProgram->CollectCableUpdates( listOfAddedCables, listOfRemovedCables );
    QList< QSharedPointer<Cable> >addedCables;
    QList<int>removedCables;
    currentContainerProgram->CollectCableUpdatesIds( &addedCables, &removedCables );

    if(removeType==RemoveType::BridgeCables)
        currentContainerProgram->CreateBridgeOverObj(objPtr->GetIndex());

    ParkObject(objPtr);

    currentContainerProgram->CollectCableUpdates();
    currentContainerProgram->CollectCableUpdatesIds();

    foreach(int id, removedCables) {

        MSGOBJ();
        msg.prop[MsgObject::Remove]=id;
        msgCtrl->SendMsg(msg);
    }

    foreach(QSharedPointer<Cable> cab, addedCables) {

        MSGOBJ();
        cab->GetInfos(msg);
        msgCtrl->SendMsg(msg);
    }

    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }

}

/*!
  Remove an object from the panel (delete it or park it)
  \param objPtr shared pointer to the object
  */
void Container::ParkObject(QSharedPointer<Object> objPtr)
{
    objPtr->ToggleEditor(false);

    //this container is not programmable : delete the object from the saved program too
    if(!listenProgramChanges) {
        foreach(ContainerProgram *prg, listContainerPrograms) {
            prg->RemoveObject(objPtr);
        }
    }

    if(currentContainerProgram)
        currentContainerProgram->RemoveObject(objPtr);

    if(objPtr==bridgeIn)
        bridgeIn.clear();
    if(objPtr==bridgeOut)
        bridgeOut.clear();
    if(objPtr==bridgeSend)
        bridgeSend.clear();
    if(objPtr==bridgeReturn)
        bridgeReturn.clear();

    listStaticObjects.removeAll(objPtr);
}

//void Container::SendMsg(const ConnectionInfo &senderPin,const PinMessage::Enum msgType,void *data)
//{
//    if(currentContainerProgram)
//        currentContainerProgram->SendMsg(senderPin,msgType,data);
//}

/*!
  Copy cables from an object
  \param newObjPtr the new object
  \param ObjPtr the object to copy
  */
void Container::CopyCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->CopyCablesFromObj( newObjPtr->GetIndex(), ObjPtr->GetIndex() );
}

void Container::MoveOutputCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->MoveOutputCablesFromObj( newObjPtr->GetIndex(), ObjPtr->GetIndex() );
}

void Container::MoveInputCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->MoveInputCablesFromObj( newObjPtr->GetIndex(), ObjPtr->GetIndex() );
}

void Container::GetListOfConnectedPinsTo(const ConnectionInfo &pin, QList<ConnectionInfo> &list)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->GetListOfConnectedPinsTo(pin,list);
}

/*!
  Called by ContainerProgam, move a parked object to the panel
  \param objPtr shared pointer to the object
  */
void Container::AddChildObject(QSharedPointer<Object> objPtr)
{
//    if(!modelIndex.isValid()) {
//        LOG("index not valid");
//        return;
//    }

//    if(objPtr->modelIndex.isValid() && objPtr->modelIndex.model()==&parkModel)
//        parkModel.removeRow(objPtr->modelIndex.row());

//    QStandardItem *item = objPtr->GetFullItem();

//    myHost->GetModel()->itemFromIndex( modelIndex )->appendRow(item);
//    objPtr->modelIndex=item->index();

    if(containersParkingId!=FixedObjId::ND && objPtr->parkingId!=FixedObjId::ND) {
//        MsgObject msg(containersParkingId);
        _MSGOBJ(msg,containersParkingId);
        msg.prop[MsgObject::Remove]=objPtr->GetIndex();
        msgCtrl->SendMsg(msg);
    }

    objPtr->parkingId=FixedObjId::ND;

    if(objPtr->GetInitDelay()>0)
        myHost->objFactory->listDelayObj << objPtr->GetIndex();
    else
        myHost->objFactory->listDelayObj.removeAll(objPtr->GetIndex());



//    myHost->SetSolverUpdateNeeded();
}

/*!
  Called by ContainerProgram, remove an object from the panel
  \param objPtr shared pointer to the object
  */
bool Container::ParkChildObject(QSharedPointer<Object> objPtr)
{
    if(!objPtr)
        return false;

    myHost->objFactory->listDelayObj.removeAll(objPtr->GetIndex());

    if(containersParkingId!=FixedObjId::ND && !listStaticObjects.contains(objPtr)) {
//        MsgObject msg(containersParkingId);
        _MSGOBJ(msg,containersParkingId);
        msg.prop[MsgObject::Add]=objPtr->GetIndex();
//        msg.prop["name"]=objPtr->info().name;
        msg.prop[MsgObject::ObjInfo]=QVariant::fromValue(objPtr->info());
        msgCtrl->SendMsg(msg);
    }

//    if(objPtr->modelIndex.isValid() && objPtr->modelIndex.model()==myHost->GetModel())
//        myHost->GetModel()->removeRow(objPtr->modelIndex.row(), objPtr->modelIndex.parent());

//    QStandardItem *item = objPtr->GetParkingItem();
//    parkModel.invisibleRootItem()->appendRow(item);
//    objPtr->modelIndex=item->index();
    objPtr->parkingId=containersParkingId;

//    myHost->SetSolverUpdateNeeded();

    return true;
}

/*!
  Called by the Object destructor, remove the object
  \param obj pointer to the object
  */
void Container::OnChildDeleted(QSharedPointer<Object>obj)
{
    listLoadedObjects.removeAll(obj);

//    if(!obj->modelIndex.isValid())
//        return;

//    if(obj->parked) {
//        if(obj->modelIndex.model()==&parkModel)
//            parkModel.removeRow(obj->modelIndex.row());
//    } else {
//        if(obj->modelIndex.model()==myHost->GetModel())
//            myHost->GetModel()->removeRow(obj->modelIndex.row(), obj->modelIndex.parent());
//    }
}

void Container::RemoveCable(QVariant pinFrom, QVariant pinTo)
{
    Pin* opinFrom = static_cast<Pin*>(qvariant_cast<QObject *>(pinFrom));
    Pin* opinTo = static_cast<Pin*>(qvariant_cast<QObject *>(pinTo));

    if(!opinFrom || !opinTo)
        return;

    UserRemoveCable(opinFrom->GetConnectionInfo(), opinTo->GetConnectionInfo());
    UserRemoveCable(opinTo->GetConnectionInfo(), opinFrom->GetConnectionInfo());
}

void Container::AddCable(QVariant pinFrom, QVariant pinTo)
{
    Pin* opinFrom = static_cast<Pin*>(qvariant_cast<QObject *>(pinFrom));
    Pin* opinTo = static_cast<Pin*>(qvariant_cast<QObject *>(pinTo));

    if(!opinFrom || !opinTo)
        return;

    UserAddCable( opinFrom->GetConnectionInfo(), opinTo->GetConnectionInfo());
}

void Container::UserAddCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin)
{
    QList< QSharedPointer<Cable> >addedCables;
    currentContainerProgram->CollectCableUpdatesIds( &addedCables, 0 );

    AddCable(outputPin,inputPin,false);

    currentContainerProgram->CollectCableUpdatesIds();
    foreach(QSharedPointer<Cable> cab, addedCables) {

        MSGOBJ();
        cab->GetInfos(msg);
        msgCtrl->SendMsg(msg);
    }

    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }
}

void Container::UserAddCable(const QPair<ConnectionInfo,ConnectionInfo>&pair)
{
    UserAddCable(pair.first,pair.second);
}

void Container::UserRemoveCableFromPin(const ConnectionInfo &pin)
{
    QList<int>removedCables;
    currentContainerProgram->CollectCableUpdatesIds( 0, &removedCables );
    RemoveCableFromPin(pin);
    currentContainerProgram->CollectCableUpdatesIds();

    foreach(int id, removedCables) {
        MSGOBJ();
        msg.prop[MsgObject::Remove]=id;
        msgCtrl->SendMsg(msg);
    }

    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }
}

void  Container::UserRemoveCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin)
{
    QList<int>removedCables;
    currentContainerProgram->CollectCableUpdatesIds( 0, &removedCables );
    RemoveCable(outputPin,inputPin);
    currentContainerProgram->CollectCableUpdatesIds();

    foreach(int id, removedCables) {

        MSGOBJ();
        msg.prop[MsgObject::Remove]=id;
        msgCtrl->SendMsg(msg);
    }

    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }
}

void Container::UserRemoveCable(const QPair<ConnectionInfo,ConnectionInfo>&pair)
{
    UserRemoveCable(pair.first,pair.second);
}

/*!
  Add a cable in this container
  \param outputPin the output pin (messages sender)
  \param inputPin the input pin (messages receiver)
  \param hidden true if the cable is hidden (the cables between containers are hidden)
  */
void Container::AddCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin, bool hidden)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->AddCable(outputPin,inputPin, hidden);
}

/*!
  Remove a cable
  \param outputPin the output pin (messages sender)
  \param inputPin the input pin (messages receiver)
  */
void Container::RemoveCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->RemoveCable(outputPin,inputPin);
}

/*!
  Remove all cables from a Pin
  \param pin the pin to disconnect
  */
void Container::RemoveCableFromPin(const ConnectionInfo &pin)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->RemoveCableFromPin(pin);
}

void Container::fromJson(QJsonObject &json)
{
    //clear programs
    foreach(ContainerProgram *prog, listContainerPrograms) {
        delete prog;
    }
    listContainerPrograms.clear();

    Object::fromJson(json);

    LoadProgram(TEMP_PROGRAM);

    if (json.contains("objects") && json["objects"].isArray()) {
        QJsonArray objArray = json["objects"].toArray();
        for (int i = 0; i < objArray.size(); ++i) {
            QJsonObject jsonObj = objArray[i].toObject();

            QJsonObject jInfo = jsonObj["objInfo"].toObject();
            ObjectInfo info( jInfo );
            info.forcedObjId=0;
            QSharedPointer<Object> objPtr = myHost->objFactory->NewObject(info);
            objPtr->fromJson( jsonObj );
            listLoadingObjects << objPtr;

            if(!objPtr->GetSleep()) {
                AddParkedObject(objPtr);
            }
        }
    }

    if (json.contains("progs") && json["progs"].isArray()) {
        QJsonArray progArray = json["progs"].toArray();
        for (int i = 0; i < progArray.size(); ++i) {
            QJsonObject prg = progArray[i].toObject();
            int savedId=0;
            ContainerProgram *prog = new ContainerProgram(myHost,this,prg,savedId);

            if(listContainerPrograms.contains(savedId))
                delete listContainerPrograms.take(savedId);
            listContainerPrograms.insert(savedId,prog);
        }
    }

    //load the default program
    LoadProgram(0);

    //clear the loading list : delete unused objects
    listLoadingObjects.clear();
    delete listContainerPrograms.take(TEMP_PROGRAM);

}

void Container::toJson(QJsonObject &json) const
{
    Object::toJson(json);

    //save all loaded objects
    QJsonArray objArray;
    foreach(QSharedPointer<Object>obj, listLoadedObjects) {
        if(obj) {
            QJsonObject jsonObj;
            obj->toJson(jsonObj);
            objArray.append(jsonObj);
        }
    }
    json["objects"] = objArray;

    //save all programs
    QJsonArray progArray;
    QHash<int,ContainerProgram*>::const_iterator i = listContainerPrograms.constBegin();
    while(i!=listContainerPrograms.constEnd()) {
        QJsonObject jProg;
        i.value()->toJson(jProg,i.key());
        progArray.append(jProg);
        ++i;
    }
    json["progs"] = progArray;
}

/*!
  Put all the ContainerProgram and children Objects in a data stream
  */
QDataStream & Container::toStream (QDataStream& out) const
{

    {
        QByteArray tmpBa;
        QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);

        //save container header
        tmpStream << (qint32)GetIndex();
        tmpStream << objectName();
        tmpStream << GetSleep();
        ProjectFile::SaveChunk( "CntHead", tmpBa, out);
    }

    //save all loaded objects
    foreach(QSharedPointer<Object>obj, listLoadedObjects) {
        if(obj) {
            QByteArray tmpBa;
            QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
            tmpStream << obj->info();
            obj->toStream( tmpStream );
            ProjectFile::SaveChunk( "CntObj", tmpBa, out);
        }
    }

    //save all programs
    QHash<int,ContainerProgram*>::const_iterator i = listContainerPrograms.constBegin();
    while(i!=listContainerPrograms.constEnd()) {
        QByteArray tmpBa;
        QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
        tmpStream << (quint32)i.key();
        tmpStream << *(i.value());
        ProjectFile::SaveChunk( "CntProg", tmpBa, out);
        ++i;
    }

    return out;
}

/*!
  Read the container from a data stream, creates all the ContainerProgram and children Objects
  */
bool Container::fromStream (QDataStream& in)
{
    //clear programs
    foreach(ContainerProgram *prog, listContainerPrograms) {
        delete prog;
    }
    listContainerPrograms.clear();

    LoadProgram(TEMP_PROGRAM);

    QString chunkName;
    QByteArray tmpBa;

    while( !in.atEnd() ) {
        QDataStream tmpStream( &tmpBa , QIODevice::ReadWrite);
        ProjectFile::LoadNextChunk( chunkName, tmpBa, in );

//        LOG(chunkName);

        if(chunkName=="CntHead")
            loadHeaderStream(tmpStream);

        else if(chunkName=="CntObj")
            loadObjectFromStream(tmpStream);

        else if(chunkName=="CntProg")
            loadProgramFromStream(tmpStream);

        //unknown chunk
        else {
            in.setStatus(QDataStream::ReadCorruptData);
            LOG("unknown section"<<chunkName<<tmpBa.size());
        }

        if(!tmpStream.atEnd()) {
            in.setStatus(QDataStream::ReadCorruptData);
#ifndef QT_NO_DEBUG
            LOG("stream not at end"<<chunkName<<"drop remaining data :");
            while(!tmpStream.atEnd()) {
                char c[1000];
                int nb=tmpStream.readRawData(c,1000);
                LOG(nb << QByteArray::fromRawData(c,nb).toHex());
            }
#endif
        }

        if(tmpStream.status()==QDataStream::ReadPastEnd) {
            LOG("ReadPastEnd"<<tmpStream.status());
            return false;
        }
    }

    //load the default program
    LoadProgram(0);

    //clear the loading list : delete unused objects
    listLoadingObjects.clear();
    delete listContainerPrograms.take(TEMP_PROGRAM);

    return true;
}

bool Container::loadHeaderStream (QDataStream &in)
{
    //load header
    qint32 id;
    in >> id;
    savedIndex = id;

    QString name;
    in >> name;
    setObjectName(name);

    bool s;
    in >> s;
    SetSleep(s);

    return true;
}

bool Container::loadObjectFromStream (QDataStream &in)
{
    ObjectInfo info;
    in >> info;
    info.forcedObjId=0;

    QSharedPointer<Object> objPtr = myHost->objFactory->NewObject(info);

    //can't create the object ?
//    if(!objPtr) {
//        LOG("object not created");
//        return false;
//    }

    if(objPtr->fromStream( in ))
        listLoadingObjects << objPtr; //keep the object alive while loading

    if(objPtr->GetSleep()) {
//        listObjects.remove(objId);
//        sharedObj.clear();
        return false;
    }

    AddParkedObject(objPtr);
    return true;
}

bool Container::loadProgramFromStream (QDataStream &in)
{
    quint32 progId;
    in >> progId;

    ContainerProgram *prog = new ContainerProgram(myHost,this);
    in >> *prog;

    if(listContainerPrograms.contains(progId))
        delete listContainerPrograms.take(progId);
    listContainerPrograms.insert(progId,prog);

    return true;
}

void Container::ProgramToStream (int progId, QDataStream &out)
{
    ContainerProgram *prog = 0;

    if(progId == currentProgId)
        prog = currentContainerProgram;
    else
        prog = listContainerPrograms.value(progId,0);

    if(!prog) {
        out << (quint8)0;
        LOG("prog not saved")
        return;
    }
    out << (quint8)1;

    out << (quint8)IsDirty();

    qint32 nbObj = prog->listObjects.size();
    out << nbObj;
    foreach(QSharedPointer<Object>obj, prog->listObjects) {
        QByteArray tmpBa;
        QDataStream tmpStream( &tmpBa , QIODevice::ReadWrite);
        if(obj) {
            tmpStream << obj->info();
            obj->ProgramToStream( progId, tmpStream );
        }
        out << tmpBa;
    }

    out << *prog;
}

void Container::ProgramFromStream (int progId, QDataStream &in)
{
    quint8 valid=0;
    in >> valid;
    if(valid!=1) {
        LOG("program not valid")
        return;
    }

    if(listProgToRemove.contains(progId)) {
        LOG("cancel deletion"<<progId<<objectName());
        listProgToRemove.removeAll(progId);
    }

    quint8 dirty;
    in >> dirty;

    QList<QSharedPointer<Object> >tmpListObj;

    qint32 nbObj;
    in >> nbObj;
    for(qint32 i=0; i<nbObj; i++) {
        QByteArray tmpBa;
        QDataStream tmpStream( &tmpBa , QIODevice::ReadWrite);
        in >> tmpBa;
        ObjectInfo info;
        tmpStream >> info;

        QSharedPointer<Object> obj;
        if(info.forcedObjId!=0) {
            obj = myHost->objFactory->GetObjectFromId( info.forcedObjId );
        }
        if(!obj) {
            obj = myHost->objFactory->NewObject(info);
            AddParkedObject(obj);
            tmpListObj << obj;
        } else {
            obj->SetContainerId(GetIndex());
            obj->ResetSavedIndex(info.forcedObjId);
        }
        if(!obj) {
            LOG("can't create obj"<<info.id<<info.name);
        }
        if(obj) {
            obj->ProgramFromStream(progId, tmpStream);
        }
    }

    ContainerProgram *prog = new ContainerProgram(myHost,this);
    in >> *prog;

    if(progId==currentProgId) {
        if(listContainerPrograms.contains(TEMP_PROGRAM))
            delete listContainerPrograms.take(TEMP_PROGRAM);
        listContainerPrograms.insert(TEMP_PROGRAM,prog);

        LoadProgram(TEMP_PROGRAM);
        currentProgId=progId;
        if(dirty)
            currentContainerProgram->SetDirty();

    } else {

        if(listContainerPrograms.contains(progId))
            delete listContainerPrograms.take(progId);
        listContainerPrograms.insert(progId,prog);

    }
    myHost->objFactory->ResetAllSavedId();
}

void Container::GetInfos(MsgObject &msg)
{
    if(!MsgEnabled())
        return;

    Object::GetInfos(msg);

    foreach( QSharedPointer< Object >obj, listStaticObjects) {
        if(obj) {
//            MsgObject a(GetIndex());
            _MSGOBJ(a,GetIndex());
            obj->GetInfos(a);
            msg.children << a;
        }
    }

    if(currentContainerProgram)
        currentContainerProgram->GetInfos(msg);
}

qint32 Container::AddObject(const ObjectInfo &newInfo, InsertionType::Enum insertType)
{
    QSharedPointer<Object> targetObj = myHost->objFactory->GetObjectFromId( GetIndex() );
    ComAddObject *com = new ComAddObject(myHost, newInfo, targetObj.staticCast<Container>(), targetObj, insertType );
    myHost->undoStack.push( com );

    return com->GetInfo().forcedObjId;
}

QVariant Container::AddObject(QString type, QString name/*=""*/, QString id/*=""*/)
{
    ObjectInfo i;
    i.nodeType = NodeType::object;

    QFileInfo fInfo;
    fInfo.setFile( type );
    QString fileType(fInfo.suffix().toLower());
    if ( fileType=="dll" || fileType=="vst") {
        i.objType = ObjType::VstPlugin;
        i.filename = type;
        i.id = name.toInt();
    }
    if ( fileType=="vst3" ) {
        i.objType = ObjType::Vst3Plugin;
        i.filename = type;
        i.apiName = name;
    }
    if(type == "AudioInterface") {
        i.objType = ObjType::AudioInterface;
        i.apiName = name;
        i.id = id.toInt();
    }
    if(type == "MidiInterface") {
        i.objType = ObjType::MidiInterface;
        i.apiName = name;
        i.id = id.toInt();
    }
    if(type == "MidiToAutomation") {
        i.objType = ObjType::MidiToAutomation;
    }
    if(type == "MidiSender") {
        i.objType = ObjType::MidiSender;
    }
    if(type == "HostController") {
        i.objType = ObjType::HostController;
    }
    if(type == "VstAutomation") {
        i.objType = ObjType::VstAutomation;
    }
    if(type == "Script") {
        i.objType = ObjType::Script;
    }
    if(type == "Buffer") {
        i.objType = ObjType::Buffer;
    }

    qint32 objid = AddObject(static_cast<ObjectInfo>(i));
    //return myHost->scriptEngine.evaluate(QString("Obj%1").arg(objid));
    return QString("Obj%1").arg(objid);
}

bool Container::RemoveObject(QVariant obj)
{
    // Object* pobj = static_cast<Object*>(obj.toQObject());
    Object* pobj = obj.value<Object*>();
    if(!pobj)
        return false;

//    MSGOBJ();
    MsgObject msg(pobj->GetIndex());
//    msg.prop[MsgObject::Remove]=RemoveType::BridgeCables;
    msg.prop[MsgObject::Remove]=pobj->GetIndex();
    myHost->SendMsg(msg);

//    QSharedPointer<Object> obj = myHost->objFactory->GetObjectFromId( id );
//    if(!obj)
//        return false;
//    ComRemoveObject *com = new ComRemoveObject(myHost, obj, RemoveType::RemoveWithCables);
//    myHost->undoStack.push( com );
    return true;
}

void Container::ReceiveMsg(const MsgObject &msg)
{


    if(msg.prop.contains(MsgObject::ConnectPin)) {
        ConnectionInfo cPin1(msg.children.value(0));
        ConnectionInfo cPin2(msg.children.value(1));
        myHost->undoStack.push( new ComAddCable(myHost, cPin1, cPin2) );
        return;
    }

    if(msg.prop.contains(MsgObject::UnplugPin)) {
        ConnectionInfo cPin(msg);
        myHost->undoStack.push( new ComDisconnectPin(myHost, cPin) );
        return;
    }



    if(msg.prop.contains(MsgObject::ObjectsToLoad)) {

        InsertionType::Enum insertType = static_cast<InsertionType::Enum>( msg.prop[MsgObject::Type].toInt() );

        QByteArray ba = msg.prop[MsgObject::ObjectsToLoad].toByteArray();
        QDataStream streamObj(&ba, QIODevice::ReadOnly);
        while(!streamObj.atEnd()) {
            ObjectInfo newInfo;
            newInfo.fromStream(streamObj);
            AddObject(newInfo, insertType);
        }

        return;
    }

    if(msg.prop.contains(MsgObject::Update)) {
//        MsgObject ans(FixedObjId::mainContainer);
//        GetInfos(ans);
//        msgCtrl->SendMsg(ans);
        SetMsgEnabled( msg.prop[MsgObject::Update].toBool() );
        UpdateView();
        return;
    }

    if(msg.prop.contains(MsgObject::FilesToLoad)) {
        QStringList lstFiles = msg.prop[MsgObject::FilesToLoad].toStringList();
        foreach(const QString filename, lstFiles) {
            if(filename.endsWith("fxb", Qt::CaseInsensitive) || filename.endsWith("fxp", Qt::CaseInsensitive)) {
                InsertionType::Enum insertType = static_cast<InsertionType::Enum>( msg.prop[MsgObject::Type].toInt() );
                QSharedPointer<Object> targetObj = myHost->objFactory->GetObjectFromId( GetIndex() );

                ObjectInfo infoVst;
                infoVst.nodeType = NodeType::object;
                infoVst.objType = ObjType::VstPlugin;
                infoVst.filename = filename;
                infoVst.name = filename;

                AddObject(infoVst, insertType);
                continue;
            }
        }
        return;
    }

    Object::ReceiveMsg(msg);
}

void Container::SetMsgEnabled(bool enab)
{
    if(enab==MsgEnabled())
        return;

    MsgHandler::SetMsgEnabled(enab);

    if(GetIndex()==FixedObjId::mainContainer)
        return;

    foreach( QSharedPointer< Object >obj, listStaticObjects) {
        if(obj) {
            obj->SetMsgEnabled(enab);
        }
    }

    if(currentContainerProgram)
        currentContainerProgram->SetMsgEnabled(enab);

    Object::SetMsgEnabled(enab);
}

QStandardItem * Container::GetModel()
{
    QStandardItem *i = new QStandardItem(objectName());

    foreach( QSharedPointer< Object >obj, listStaticObjects) {
        if(obj) {
            i->appendRow( obj->GetModel() );
        }
    }

    foreach( QSharedPointer< Object >obj, listLoadedObjects) {
        if(obj) {
            i->appendRow( obj->GetModel() );
        }
    }
    QStandardItem *cables = new QStandardItem("Cables");
    if(currentContainerProgram) {
        foreach(QSharedPointer<Cable>cab, currentContainerProgram->listCables) {
            Connectables::Pin *pinO = myHost->objFactory->GetPin(cab->GetInfoOut());
            Connectables::Pin *pinI = myHost->objFactory->GetPin(cab->GetInfoIn());
            QSharedPointer<Object> On = myHost->objFactory->GetObjectFromId(cab->GetInfoOut().objId);
            QSharedPointer<Object> In = myHost->objFactory->GetObjectFromId(cab->GetInfoIn().objId);

            QString n = QString("%1.%2->%3.%4 (%5->%6)")
                    .arg(On->scriptName)
                    .arg(pinO->objectName())
                    .arg(In->scriptName)
                    .arg(pinI->objectName())
                    .arg(On->objectName())
                    .arg(In->objectName());
            QStandardItem *c = new QStandardItem(n);
            cables->appendRow(c);
        }
    }
    if(cables->rowCount()!=0)
        i->appendRow(cables);
    return i;
}
