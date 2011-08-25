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
#include "connectables/objectfactory.h"
#include "mainhost.h"
#include "projectfile/projectfile.h"
#include "models/programsmodel.h"

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
Container::Container(MainHost *myHost, MetaInfo &info) :
    Object(myHost, info ),
    bridgeIn(0),
    bridgeOut(0),
    bridgeSend(0),
    bridgeReturn(0),
    optimizerFlag(false),
    currentContainerProgram(0),
    progToSet(-1),
    loadingMode(false)
{
    SetType(MetaTypes::container);

    SET_MUTEX_NAME(mutexCurrentProg,"mutexCurrentProg"+Name())

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

///*!
//  Get the cables model node (cables are in a child node)
//  \return the node or an invalid index
//  */
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

/*!
  Connect the output pins of fromObjOutputs to the inputs of toObjInputs
  \param fromObjOutputs
  \param toObjInputs
  \param hidden true to create hidden cables
  */
void Container::ConnectObjects(QSharedPointer<Object> fromObjOutputs, QSharedPointer<Object> toObjInputs, bool hiddenCables)
{
    if(fromObjOutputs.isNull() || toObjInputs.isNull())
        return;

    PinsList *lstOut=0;
    PinsList *lstIn=0;

    QList<MediaTypes::Enum>lstType;
    lstType << MediaTypes::Audio << MediaTypes::Midi << MediaTypes::Bridge;

    foreach(MediaTypes::Enum t, lstType) {
        lstOut = fromObjOutputs->GetPinList(Directions::Output,t);
        lstIn = toObjInputs->GetPinList(Directions::Input,t);
        if(lstOut && lstIn)
            lstOut->ConnectAllTo(this,lstIn, hiddenCables);
    }
}

bool Container::Close()
{
    if(closed)
        return false;

    foreach(QSharedPointer<Object> objPtr, listStaticObjects) {
        if(objPtr.isNull())
            continue;
        ParkObject(objPtr);
    }
    listStaticObjects.clear();

    mutexCurrentProg.lock();
    if(currentContainerProgram) {
        currentContainerProgram->Unload();
        currentContainerProgram->ParkAllObj();
        delete currentContainerProgram;
    }
    qDeleteAll(listContainerPrograms);
    mutexCurrentProg.unlock();

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
        if(!objPtr.isNull())
            objPtr->SetSleep(sleep);
    }
}

void Container::Hide()
{
    if(currentContainerProgram) {
        foreach(QSharedPointer<Object> objPtr, currentContainerProgram->listObjects) {
            if(!objPtr.isNull())
                objPtr->Hide();
        }
    }
    foreach(QSharedPointer<Object> objPtr, listStaticObjects) {
        if(!objPtr.isNull())
            objPtr->Hide();
    }

    Object::Hide();

}

/*!
  Will change program on the next render loop
  \param prg a program model index
  */
void Container::SetProgram(const QModelIndex &idx, bool fromCommand)
{
//    if(!fromCommand)
//        if(!mutexProgChange.tryLock())
//            return;

    progToSet=idx.data(ProgramsModel::ProgramId).toInt();
    if(progToSet == currentProgId) {
        progToSet=-1;
//        if(!fromCommand)
//            mutexProgChange.unlock();
        return;
    }

    LoadProgram(progToSet);
    progToSet=-1;
//    if(!fromCommand)
//        mutexProgChange.unlock();
}

void Container::NewRenderLoop()
{
    Object::NewRenderLoop();

    foreach(QSharedPointer<Object> obj, listStaticObjects) {
        obj->NewRenderLoop();
    }
}


void Container::SetBufferSize(unsigned long size)
{
    foreach(Object *obj, listLoadedObjects) {
        obj->SetBufferSize(size);
    }
}

void Container::SetSampleRate(float rate)
{
    foreach(Object *obj, listLoadedObjects) {
        obj->SetSampleRate(rate);
    }
}

void Container::LoadProgram(int prog)
{
    //if prog is already loaded
    if(prog==currentProgId && currentContainerProgram) {
        return;
    }

    LOG("prog"<<prog);

//    QMutexLocker l(&mutexCurrentProg);

    ContainerProgram *oldProg = currentContainerProgram;
    ContainerProgram *newProg  = 0;

    if(prog==TEMP_PROGRAM || prog==EMPTY_PROGRAM) {
        newProg = new ContainerProgram(myHost,this);
    } else {
        if(listContainerPrograms.contains(prog)) {
            newProg = listContainerPrograms.value(prog);
        } else {
            newProg = new ContainerProgram(myHost,this);
            listContainerPrograms.insert(prog,newProg);
        }
    }

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

    if(optimizerFlag)
        currentContainerProgram->LoadRendererState();

    if(oldProg) {
        mutexCurrentProg.lock();
        delete oldProg;
        mutexCurrentProg.unlock();
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
    LOG("saveprog"<<currentProgId);

    if(!currentContainerProgram && currentProgId==TEMP_PROGRAM)
        return;

    currentContainerProgram->Save();

    mutexCurrentProg.lock();
    delete listContainerPrograms.take(currentProgId);
    mutexCurrentProg.unlock();

    listContainerPrograms.insert(currentProgId,currentContainerProgram);
    ContainerProgram *tmp = new ContainerProgram(*currentContainerProgram);
    currentContainerProgram = tmp;
}

void Container::UnloadProgram()
{
    if(!currentContainerProgram)
        return;

    currentContainerProgram->Unload();

    mutexCurrentProg.lock();
    delete currentContainerProgram;
    mutexCurrentProg.unlock();

    currentContainerProgram=0;

    currentProgId=EMPTY_PROGRAM;
}

/*!
  Try to remove the program now, retry later if we try to remove the current program
  */
void Container::RemoveProgram(int progId)
{
    if(progId>0) {
//        LOG("add prog to remove progId"<<progId<<QThread::currentThread());
        listProgToRemove << progId;
    }

    QList<int>remainingProgs;

    while(!listProgToRemove.isEmpty()) {
        int p = listProgToRemove.takeFirst();

        if(listContainerPrograms.contains(p)) {
            if(p == currentProgId) {
                if(progToSet==-1) {
                    LOG("removing current program and no scheduled progChange, prog not deleted "<<p<<objectName());
                } else {
                    remainingProgs << p;
                }
            } else {
                mutexCurrentProg.lock();
                delete listContainerPrograms.take(p);
                mutexCurrentProg.unlock();
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
    objPtr->SetContainer(this);
    objPtr->UnloadProgram();

    //bridges are not stored in program
    int direction = objPtr->Meta(MetaInfos::Direction).toInt();
    if(objPtr->Type() == MetaTypes::bridge) {
        switch(direction) {
        case Directions::Input :
            bridgeIn=objPtr;
            break;
        case Directions::Output :
            bridgeOut=objPtr;
            break;
        case Directions::Send :
            bridgeSend=objPtr;
            break;
        case Directions::Return :
            bridgeReturn=objPtr;
        }

        objPtr->listenProgramChanges=false;
        listStaticObjects << objPtr;
        AddChildObject(objPtr);
        objPtr->LoadProgram(currentProgId);
        return;
    }

    if(!listLoadedObjects.contains(objPtr.data()))
        listLoadedObjects << objPtr.data();
    currentContainerProgram->AddObject(objPtr);
    objPtr->LoadProgram(currentProgId);
}

/*!
  Place an object and park it
  \param objPtr shared pointer to the object
  */
void Container::AddParkedObject(QSharedPointer<Object> objPtr)
{
    objPtr->SetContainer(this);
    objPtr->UnloadProgram();

    listLoadedObjects << objPtr.data();
    ParkChildObject(objPtr);
}

void Container::UserAddObject(const QSharedPointer<Object> &objPtr,
                              InsertionType::Enum insertType,
                              QList< QPair<MetaInfo,MetaInfo> > *listOfAddedCables,
                              QList< QPair<MetaInfo,MetaInfo> > *listOfRemovedCables,
                              const QSharedPointer<Object> &targetPtr)
{
    LOG("useradd"<<currentProgId<<objPtr->Name());
    AddObject(objPtr);

    if(targetPtr) {
        currentContainerProgram->CollectCableUpdates( listOfAddedCables, listOfRemovedCables );

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
            case InsertionType::Replace:
                CopyCablesFromObj(objPtr, targetPtr);
                (targetPtr)->CopyStatusTo(objPtr);
                ParkObject(targetPtr);
                break;
            case InsertionType::NoInsertion:
                break;
        }
        currentContainerProgram->CollectCableUpdates();
    }
    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }
}


void Container::UserParkObject(QSharedPointer<Object> objPtr,
                               RemoveType::Enum removeType,
                               QList< QPair<MetaInfo,MetaInfo> > *listOfAddedCables,
                               QList< QPair<MetaInfo,MetaInfo> > *listOfRemovedCables)
{
    currentContainerProgram->CollectCableUpdates( listOfAddedCables, listOfRemovedCables );

    if(removeType==RemoveType::BridgeCables)
        currentContainerProgram->CreateBridgeOverObj(objPtr->ObjId());

    ParkObject(objPtr);

    currentContainerProgram->CollectCableUpdates();

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

/*!
  Copy cables from an object
  \param newObjPtr the new object
  \param ObjPtr the object to copy
  */
void Container::CopyCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->CopyCablesFromObj( newObjPtr->ObjId(), ObjPtr->ObjId());
}

void Container::MoveOutputCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->MoveOutputCablesFromObj( newObjPtr->ObjId(), ObjPtr->ObjId() );
}

void Container::MoveInputCablesFromObj(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> ObjPtr)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->MoveInputCablesFromObj( newObjPtr->ObjId(), ObjPtr->ObjId() );
}

void Container::GetListOfConnectedPinsTo(const MetaInfo &pin, QList<MetaInfo> &list)
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
    objPtr->parked=false;
    objPtr->Resume();
    objPtr->SetParent(this);
    LOG("addchild"<<currentProgId<<objPtr->Name());
    objPtr->AddToView();
}

/*!
  Called by ContainerProgram, remove an object from the panel
  \param objPtr shared pointer to the object
  */
void Container::ParkChildObject(QSharedPointer<Object> objPtr)
{
    LOG("parkchild"<<currentProgId<<objPtr->Name());
    if(objPtr.isNull())
        return;

//    if(objPtr->Type()==MetaTypes::object) {
        objPtr->parked=true;
        objPtr->AddToParkView();
        QTimer::singleShot(50,objPtr.data(),SLOT(SuspendIfParked()));
//    }
}

/*!
  Called by the Object destructor, remove the object
  \param obj pointer to the object
  */
void Container::OnChildDeleted(Object *obj)
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
    obj->RemoveFromView();
}

void Container::UserAddCable(const MetaInfo &outputPin, const MetaInfo &inputPin)
{
    AddCable(outputPin,inputPin,false);

    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }
}

void Container::UserAddCable(const QPair<MetaInfo,MetaInfo>&pair)
{
    UserAddCable(pair.first,pair.second);
}

void Container::UserRemoveCableFromPin(const MetaInfo &pin)
{
    RemoveCableFromPin(pin);

    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }
}

void  Container::UserRemoveCable(const MetaInfo &outputPin, const MetaInfo &inputPin)
{
    RemoveCable(outputPin,inputPin);

    if(!loadingMode) {
        myHost->SetSolverUpdateNeeded();
        UpdateModificationTime();
    }
}

void Container::UserRemoveCable(const QPair<MetaInfo,MetaInfo>&pair)
{
    UserRemoveCable(pair.first,pair.second);
}

/*!
  Add a cable in this container
  \param outputPin the output pin (messages sender)
  \param inputPin the input pin (messages receiver)
  \param hidden true if the cable is hidden (the cables between containers are hidden)
  */
void Container::AddCable(const MetaInfo &outputPin, const MetaInfo &inputPin, bool hidden)
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
void Container::RemoveCable(const MetaInfo &outputPin, const MetaInfo &inputPin)
{
    if(!currentContainerProgram)
        return;
    currentContainerProgram->RemoveCable(outputPin,inputPin);
}

/*!
  Remove all cables from a Pin
  \param pin the pin to disconnect
  */
void Container::RemoveCableFromPin(const MetaInfo &pin)
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
        tmpStream << (qint32)MetaInfo::ObjId();
        tmpStream << objectName();
        tmpStream << sleep;
//        LOG(tmpBa.size() << tmpBa.toHex());
        ProjectFile::SaveChunk( "CntHead", tmpBa, out);
    }

    //save all loaded objects
    foreach(Object* obj, listLoadedObjects) {
        QByteArray tmpBa;
        QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
        tmpStream << obj->info();
        obj->toStream( tmpStream );
        ProjectFile::SaveChunk( "CntObj", tmpBa, out);
    }

    //save all programs
    QHash<int,ContainerProgram*>::const_iterator i = listContainerPrograms.constBegin();
    while(i!=listContainerPrograms.constEnd()) {
        QByteArray tmpBa;
        QDataStream tmpStream( &tmpBa, QIODevice::ReadWrite);
        tmpStream << (quint16)i.key();
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
    qDeleteAll(listContainerPrograms);
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
            LOG("ReadPastEnd"<<tmpStream.status()<<toStringFull());
            return false;
        }
    }

    //load the default program
    LoadProgram(0);

    //clear the loading list : delete unused objects
    listLoadingObjects.clear();
    delete listContainerPrograms.take(TEMP_PROGRAM);

    MetaInfo::ResetSavedIds();

    return true;
}

bool Container::loadHeaderStream (QDataStream &in)
{
    //load header
    quint32 id;
    in >> id;
    MetaInfo::SetSavedId(id,ObjId());

    QString name;
    in >> name;
    SetName(name);

    in >> sleep;

    return true;
}

bool Container::loadObjectFromStream (QDataStream &in)
{
    MetaInfo info;
    in >> info;
    quint32 oldId=info.ObjId();
    info.SetObjId(0);

    QSharedPointer<Object> objPtr = myHost->objFactory->NewObject(info);

    //error while creating the object, build a dummy object with the same saved id
    if(objPtr.isNull()) {
        objPtr = myHost->objFactory->NewObject(info);
    }

    //can't even create a dummy object ?
    if(objPtr.isNull()) {
        LOG("dummy object not created");
    }

    if(!objPtr.isNull()) {
        MetaInfo::SetSavedId(oldId,objPtr->ObjId());
        AddParkedObject(objPtr);
        if(objPtr->fromStream( in ))
            listLoadingObjects << objPtr; //keep the object alive while loading
    }

    return true;
}

bool Container::loadProgramFromStream (QDataStream &in)
{
    quint16 progId;
    in >> progId;

    ContainerProgram *prog = new ContainerProgram(myHost,this);
    in >> *prog;

    if(listContainerPrograms.contains(progId)) {
        mutexCurrentProg.lock();
        delete listContainerPrograms.take(progId);
        mutexCurrentProg.unlock();
    }
    listContainerPrograms.insert(progId,prog);

    return true;
}

void Container::ProgramToStream (int progId, QDataStream &out)
{
//    LOG("prg to stream"<<progId<<QThread::currentThread())

    ContainerProgram *prog = 0;

    mutexCurrentProg.lock();
    if(progId == currentProgId)
        prog = new ContainerProgram(*currentContainerProgram);
    else if(listContainerPrograms.contains(progId))
        prog = new ContainerProgram(*listContainerPrograms.value(progId));
    mutexCurrentProg.unlock();

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
    delete prog;
}

void Container::ProgramFromStream (int progId, QDataStream &in)
{
    quint8 valid=0;
    in >> valid;
    if(valid!=1)
        return;

//    LOG("prg from stream"<<progId<<QThread::currentThread())

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
        MetaInfo info;
        tmpStream >> info;
        QSharedPointer<Object>obj = myHost->objFactory->GetObjectFromId( info.ObjId() );
        if(!obj) {
            obj = myHost->objFactory->NewObject(info);
            AddParkedObject(obj);
            tmpListObj << obj;
            MetaInfo::SetSavedId(info.ObjId(),obj->ObjId());
        } else {
            MetaInfo::SetSavedId(info.ObjId(),info.ObjId());
            obj->SetContainer(this);
        }
        if(!obj) {
            LOG("can't create obj");
        }
        if(obj) {
            obj->ProgramFromStream(progId, tmpStream);
        }
    }

    ContainerProgram *prog = new ContainerProgram(myHost,this);
    in >> *prog;

    if(progId==currentProgId) {
        mutexCurrentProg.lock();
        if(listContainerPrograms.contains(TEMP_PROGRAM)) {
            delete listContainerPrograms.take(TEMP_PROGRAM);
        }
        listContainerPrograms.insert(TEMP_PROGRAM,prog);
        mutexCurrentProg.unlock();

        LoadProgram(TEMP_PROGRAM);
        currentProgId=progId;
        if(dirty)
            currentContainerProgram->SetDirty();

    } else {
        mutexCurrentProg.lock();
        if(listContainerPrograms.contains(progId)) {
            delete listContainerPrograms.take(progId);
        }
        listContainerPrograms.insert(progId,prog);
        mutexCurrentProg.unlock();
    }

    MetaInfo::ResetSavedIds();
}
