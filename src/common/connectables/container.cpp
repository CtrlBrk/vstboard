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
#include "container.h"
#include "objectfactory.h"
#include "mainhost.h"
#include "projectfile/projectfile.h"
#include "models/programsmodel.h"

#include "commands/comaddobject.h"
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
    optimizerFlag(false),
    currentContainerProgram(0),
    cablesNode(QModelIndex()),
    progToSet(-1),
    loadingMode(false),
    containersParkingId(FixedObjId::ND)
{
    qDeleteAll(pinLists);
    pinLists.clear();
    listAudioPinIn=0;
    listAudioPinOut=0;
    listParameterPinIn=0;
    listParameterPinOut=0;
    listMidiPinIn=0;
    listMidiPinOut=0;
    listBridgePinIn=0;
    listBridgePinOut=0;

    parkModel.setObjectName("parkModel"%objectName());
    LoadProgram(TEMP_PROGRAM);
    connect(myHost,SIGNAL(BufferSizeChanged(ulong)),
            this,SLOT(SetBufferSize(ulong)));
    connect(myHost,SIGNAL(SampleRateChanged(float)),
            this,SLOT(SetSampleRate(float)));
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

void Container::SetContainerId(quint16 id)
{
    Object::SetContainerId(id);

    foreach(QSharedPointer<Object>obj, listLoadedObjects) {
        if(obj)
            obj->SetContainerId(GetIndex());
    }
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

    fromObjOutputs->GetListAudioPinOut()->ConnectAllTo(this,toObjInputs->GetListAudioPinIn(), hiddenCables);
    fromObjOutputs->GetListMidiPinOut()->ConnectAllTo(this,toObjInputs->GetListMidiPinIn(), hiddenCables);
    fromObjOutputs->GetListBridgePinOut()->ConnectAllTo(this,toObjInputs->GetListBridgePinIn(), hiddenCables);
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
            objPtr->SetSleep(sleep);
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

//    if(!myHost->mutexRender.tryLock())
//        return;
//    LoadProgram(progToSet);
//    progToSet=-1;
//    myHost->mutexRender.unlock();

}

void Container::NewRenderLoop()
{
//    Object::NewRenderLoop();

//    foreach(QSharedPointer<Object> obj, listStaticObjects) {
//        obj->NewRenderLoop();
//    }
}

void Container::PostRender()
{
    if(progToSet!=-1) {
        int p =progToSet;
        progToSet=-1;

//        myHost->mutexRender.lock();
        LoadProgram(p);

//        myHost->mutexRender.unlock();
    }
}

void Container::SetBufferSize(unsigned long size)
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
//        UpdateModelNode();
        return;
    }

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

//    if(modelIndex.isValid())
//        UpdateModelNode();

    if(oldProg) {
        delete oldProg;
    }

    if(GetIndex()!=FixedObjId::mainContainer) {
        SetMsgEnabled(true);
        MsgObject msg(FixedObjId::mainContainer);
        GetInfos(msg);
        msgCtrl->SendMsg(msg);
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
    if(!currentContainerProgram && currentProgId==TEMP_PROGRAM)
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

    if(MsgEnabled()) {
        MsgObject msg(GetIndex());
        objPtr->GetInfos(msg);
        msgCtrl->SendMsg(msg);
    }

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
                    MsgObject msg(GetIndex());
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
                MsgObject msg(GetIndex());
                msg.prop[MsgObject::Remove]=id;
                msgCtrl->SendMsg(msg);
            }


            foreach(QSharedPointer<Cable> cab, addedCables) {
                MsgObject msg(GetIndex());
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
    MsgObject msg(GetIndex());
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
        MsgObject msg(GetIndex());
        msg.prop[MsgObject::Remove]=id;
        msgCtrl->SendMsg(msg);
    }

    foreach(QSharedPointer<Cable> cab, addedCables) {
        MsgObject msg(GetIndex());
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

    if(containersParkingId!=FixedObjId::ND) {
        MsgObject msg(containersParkingId);
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
void Container::ParkChildObject(QSharedPointer<Object> objPtr)
{
    if(!objPtr)
        return;

    myHost->objFactory->listDelayObj.removeAll(objPtr->GetIndex());

    if(containersParkingId!=FixedObjId::ND) {
        MsgObject msg(containersParkingId);
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
    objPtr->parkingId=parkingId;



//    myHost->SetSolverUpdateNeeded();
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

void Container::UserAddCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin)
{
    QList< QSharedPointer<Cable> >addedCables;
    currentContainerProgram->CollectCableUpdatesIds( &addedCables, 0 );

    AddCable(outputPin,inputPin,false);

    currentContainerProgram->CollectCableUpdatesIds();
    foreach(QSharedPointer<Cable> cab, addedCables) {
        MsgObject msg(GetIndex());
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
        MsgObject msg(GetIndex());
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
        MsgObject msg(GetIndex());
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

/*!
  Put all the ContainerProgram and children Objects in a data stream
  */
QDataStream & Container::toStream (QDataStream& out) const
{

    {
        QByteArray tmpBa;
        QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);

        //save container header
        tmpStream << (qint16)GetIndex();
        tmpStream << objectName();
        tmpStream << sleep;
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
    qint16 id;
    in >> id;
    savedIndex = id;

    QString name;
    in >> name;
    setObjectName(name);

    in >> sleep;

    return true;
}

bool Container::loadObjectFromStream (QDataStream &in)
{
    ObjectInfo info;
    in >> info;
    info.forcedObjId=0;

    QSharedPointer<Object> objPtr = myHost->objFactory->NewObject(info);

    //can't create the object ?
    if(!objPtr) {
        LOG("object not created");
        return false;
    }

    AddParkedObject(objPtr);
    if(objPtr->fromStream( in ))
        listLoadingObjects << objPtr; //keep the object alive while loading

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
        return;
    }
    out << (quint8)1;

    out << (quint8)IsDirty();

    quint16 nbObj = prog->listObjects.size();
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
    if(valid!=1)
        return;

    if(listProgToRemove.contains(progId)) {
        LOG("cancel deletion"<<progId<<objectName());
        listProgToRemove.removeAll(progId);
    }

    quint8 dirty;
    in >> dirty;

    QList<QSharedPointer<Object> >tmpListObj;

    quint16 nbObj;
    in >> nbObj;
    for(int i=0; i<nbObj; i++) {
        QByteArray tmpBa;
        QDataStream tmpStream( &tmpBa , QIODevice::ReadWrite);
        in >> tmpBa;
        ObjectInfo info;
        tmpStream >> info;
        QSharedPointer<Object>obj = myHost->objFactory->GetObjectFromId( info.forcedObjId );
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
    myHost->objFactory->ResetSavedId();
}

void Container::GetInfos(MsgObject &msg)
{
    if(!MsgEnabled())
        return;

    Object::GetInfos(msg);

    foreach( QSharedPointer< Object >obj, listStaticObjects) {
        if(obj) {
            MsgObject a(GetIndex());
            obj->GetInfos(a);
            msg.children << a;
        }
    }

    if(currentContainerProgram)
        currentContainerProgram->GetInfos(msg);
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

        int insertType = msg.prop[MsgObject::Type].toInt();
        QSharedPointer<Object> targetObj = myHost->objFactory->GetObjectFromId( GetIndex() );

        QDataStream streamObj(&msg.prop[MsgObject::ObjectsToLoad].toByteArray(), QIODevice::ReadOnly);
        while(!streamObj.atEnd()) {
            ObjectInfo newInfo;
            newInfo.fromStream(streamObj);
            ComAddObject *com = new ComAddObject(myHost, newInfo, targetObj.staticCast<Container>(), targetObj, static_cast<InsertionType::Enum>(insertType) );
            myHost->undoStack.push( com );
        }

        return;
    }

    if(msg.prop.contains(MsgObject::Update)) {
        MsgObject ans(FixedObjId::mainContainer);
        GetInfos(ans);
        msgCtrl->SendMsg(ans);
        return;
    }

    Object::ReceiveMsg(msg);
}

void Container::SetMsgEnabled(bool enab)
{
    Object::SetMsgEnabled(enab);

    foreach( QSharedPointer< Object >obj, listStaticObjects) {
        if(obj) {
            obj->SetMsgEnabled(enab);
        }
    }

    if(currentContainerProgram)
        currentContainerProgram->SetMsgEnabled(enab);
}
