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


#include "object.h"
#include "../globals.h"
#include "../mainhost.h"
#include "../renderer/pathsolver.h"
#include "container.h"

#include "commands/comaddobject.h"
#include "commands/comremoveobject.h"

using namespace Connectables;

/*!
    \class Connectables::Object
    \brief virtual object, children of containers, parents of pins
*/

/*!
  Constructor, used by ObjectFactory
  \param host parent MainHost
  \param index unique id
  \param info ObjectInfo defining this object
  */
Object::Object(MainHost *host, int index, const ObjectInfo &info) :
    QObject(),
    MsgHandler(host,index),
    parkingId(FixedObjId::ND),
    listenProgramChanges(true),
    myHost(host),
    listAudioPinIn(new PinsList(host, this, msgCtrl, myHost->objFactory->GetNewObjId()) ),
    listAudioPinOut(new PinsList(host, this, msgCtrl, myHost->objFactory->GetNewObjId()) ),
    listMidiPinIn(new PinsList(host, this, msgCtrl, myHost->objFactory->GetNewObjId()) ),
    listMidiPinOut(new PinsList(host, this, msgCtrl, myHost->objFactory->GetNewObjId()) ),
    listBridgePinIn(new PinsList(host, this, msgCtrl, myHost->objFactory->GetNewObjId()) ),
    listBridgePinOut(new PinsList(host, this, msgCtrl, myHost->objFactory->GetNewObjId()) ),
    listParameterPinIn(new PinsList(host, this, msgCtrl, myHost->objFactory->GetNewObjId()) ),
    listParameterPinOut(new PinsList(host, this, msgCtrl, myHost->objFactory->GetNewObjId()) ),
    solverNode(0),
    savedIndex(-2),
    sleep(true),
    currentProgram(0),
    currentProgId(TEMP_PROGRAM),
    closed(true),
    objInfo(info),
    containerId(FixedObjId::noContainer),
    initialDelay(0L)
{
    objInfo.forcedObjId = index;

    setObjectName(objInfo.name);
    doublePrecision=myHost->doublePrecision;

#ifdef SCRIPTENGINE
    QScriptValue scriptObj = myHost->scriptEngine->newQObject(this);
    myHost->scriptEngine->globalObject().setProperty(QString("Obj%1").arg(index), scriptObj);
#endif

    //init pins lists
    ConnectionInfo i;
    i.objId=index;
    i.myHost=myHost;

    i.type=PinType::Audio;
    i.direction=PinDirection::Input;
    listAudioPinIn->SetInfo(this,i,ObjectInfo(NodeType::listPin, ObjType::listAudioIn));
    listAudioPinIn->setObjectName("listAudioPinIn");
    i.direction=PinDirection::Output;
    listAudioPinOut->SetInfo(this,i,ObjectInfo(NodeType::listPin, ObjType::listAudioOut));
    listAudioPinOut->setObjectName("listAudioPinOut");

    i.type=PinType::Midi;
    i.direction=PinDirection::Input;
    listMidiPinIn->SetInfo(this,i,ObjectInfo(NodeType::listPin, ObjType::listMidiIn));
    listMidiPinIn->setObjectName("listMidiPinIn");
    i.direction=PinDirection::Output;
    listMidiPinOut->SetInfo(this,i,ObjectInfo(NodeType::listPin, ObjType::listMidiOut));
    listMidiPinOut->setObjectName("listMidiPinOut");

    i.type=PinType::Bridge;
    i.direction=PinDirection::Input;
    listBridgePinIn->SetInfo(this,i,ObjectInfo(NodeType::listPin, ObjType::listBridgeIn));
    listBridgePinIn->setObjectName("listBridgePinIn");
    i.direction=PinDirection::Output;
    listBridgePinOut->SetInfo(this,i,ObjectInfo(NodeType::listPin, ObjType::listBridgeOut));
    listBridgePinOut->setObjectName("listBridgePinOut");

    i.type=PinType::Parameter;
    i.direction=PinDirection::Input;
    listParameterPinIn->SetInfo(this,i,ObjectInfo(NodeType::listPin, ObjType::listParamIn));
    listParameterPinIn->setObjectName("listParameterPinIn");
    i.direction=PinDirection::Output;
    listParameterPinOut->SetInfo(this,i,ObjectInfo(NodeType::listPin, ObjType::listParamOut));
    listParameterPinOut->setObjectName("listParameterPinOut");

    pinLists.insert("audioin", listAudioPinIn);
    pinLists.insert("audioout", listAudioPinOut);
    pinLists.insert("midiin", listMidiPinIn);
    pinLists.insert("midiout", listMidiPinOut);
    pinLists.insert("bridgein", listBridgePinIn);
    pinLists.insert("bridgeout", listBridgePinOut);
    pinLists.insert("parameterin",listParameterPinIn);
    pinLists.insert("parameterout",listParameterPinOut);


}

/*!
  Destructor
  */
Object::~Object()
{
    pinLists.clear();

    if(containerId!=FixedObjId::noContainer) {
        QSharedPointer<Container>cntPtr = myHost->objFactory->GetObjectFromId( containerId ).staticCast<Container>();
        QSharedPointer<Object>objPtr = myHost->objFactory->GetObjectFromId( GetIndex() );
        if(cntPtr && objPtr) {
            cntPtr->OnChildDeleted(objPtr);
        }
    }
    Close();

    myHost->objFactory->RemoveObject(GetIndex());
}

/*!
  Called by ObjectFactory after the object creation
  \return true on success, false if the object should be deleted
  */
bool Object::Open()
{
    closed=false;
    return true;
}

/*!
  Cleanup before destruction
  \return false if the object is already closed
  */
bool Object::Close()
{
    if(closed)
        return false;

    SetSleep(true);

//    if(MsgEnabled()) {
        if(parkingId==FixedObjId::ND) {
//            MsgObject msg(containerId);
//            msg.prop[MsgObject::Remove]=GetIndex();
//            msgCtrl->SendMsg(msg);
        } else {
            MsgObject msg(parkingId);
            msg.prop[MsgObject::Remove]=GetIndex();
            msgCtrl->SendMsg(msg);
        }
//    }

//    Hide();
    closed=true;

    foreach(ObjectProgram *prg, listPrograms) {
        delete prg;
    }
    listPrograms.clear();

    return true;
}

/*!
  Hide the object and all its pins
  \todo not used anymore except for bridges
  */
//void Object::Hide() {
//    foreach(PinsList *lst, pinLists) {
//        lst->Hide();
//    }
//}

/*!
  Toggle visibility of input bridges pins
  \todo only used by bridges, can it be removed ?
  */
//void Object::SetBridgePinsInVisible(bool visible)
//{
//    listBridgePinIn->SetVisible(visible);
//}

/*!
  Toggle visibility of output bridges pins
  \todo only used by bridges, can it be removed ?
  */
//void Object::SetBridgePinsOutVisible(bool visible)
//{
//    listBridgePinOut->SetVisible(visible);
//}

/*!
  Set the objects name, update the view
  */
void Object::setObjectName(const QString &name)
{
//    if(modelIndex.isValid())
//        myHost->GetModel()->setData(modelIndex, name, Qt::DisplayRole);

    QObject::setObjectName(name);
}

/*!
  Toggle sleep mode, the object is not rendered when sleeping
  */
void Object::SetSleep(bool sleeping)
{
    objMutex.lock();
    sleep = sleeping;
    objMutex.unlock();
}

/*!
  Retrive current sleep state
  */
bool Object::GetSleep()
{
    QMutexLocker l(&objMutex);
    return sleep;
}

/*!
  Check if the objects program has been modified
  \return true if modified
  */
bool Object::IsDirty()
{
    if(!currentProgram)
        return false;
    return currentProgram->IsDirty();
}

/*!
  Set the current program dirty flag
  Called by ParameterPin
  */
void Object::OnProgramDirty()
{
    if(!currentProgram)
        return;

    currentProgram->SetDirty();
}

/*!
  Unload current progam
  */
void Object::UnloadProgram()
{
    foreach(PinsList *lst, pinLists) {
        lst->EnableVuUpdates(false);
    }
    currentProgram=0;
    currentProgId=EMPTY_PROGRAM;
}

/*!
  Save the current program
  */
void Object::SaveProgram()
{
    if(!currentProgram || !currentProgram->IsDirty())
        return;

    currentProgram->Save(listParameterPinIn,listParameterPinOut);
}

/*!
  Load a program
    a new program is created if needed
    drop the current program if one is loaded
    \param prog program number
  */
void Object::LoadProgram(int prog)
{
    //if prog is already loaded, update model
    if(prog==currentProgId && currentProgram) {
//        UpdateModelNode();
        return;
    }

    bool msgWasEnabled=MsgEnabled();
    SetMsgEnabled(false);

    //if a program is loaded, unload it without saving
    int progWas = currentProgId;
    if(currentProgId!=EMPTY_PROGRAM && currentProgram)
        UnloadProgram();

    currentProgId=prog;

    if(!listPrograms.contains(currentProgId))
        listPrograms.insert(currentProgId,new ObjectProgram(listParameterPinIn,listParameterPinOut));

    currentProgram=listPrograms.value(currentProgId);
    currentProgram->Load(listParameterPinIn,listParameterPinOut);

//    UpdateModelNode();
    foreach(PinsList *lst, pinLists) {
        lst->EnableVuUpdates(true);
    }

    //if the loaded program was a temporary prog, delete it
    if(progWas==TEMP_PROGRAM) {
        delete listPrograms.take(TEMP_PROGRAM);
    }

    if(msgWasEnabled)
        SetMsgEnabled(true);
}

/*!
  Remove a program from the program list
  */
void Object::RemoveProgram(int prg)
{
    if(prg == currentProgId) {
        LOG("removing current program ! "<<prg<<objectName());
        return;
    }

    if(!listPrograms.contains(prg)) {
        LOG("prog not found"<<prg);
        return;
    }
    delete listPrograms.take(prg);
}

/*!
  Prepare for a new rendering
  Called one time at the beginning of the loop
  */
void Object::NewRenderLoop()
{
    if(sleep)
        return;

    foreach(Pin *pin, listAudioPinIn->listPins) {
        pin->NewRenderLoop();
    }

    foreach(Pin *p, listBridgePinIn->listPins) {
        p->NewRenderLoop();
    }
}

/*!
  Get the pin corresponding to the info
  \param pinInfo a ConnectionInfo describing the pin
  \return a pointer to the pin, 0 if not found
  */
Pin * Object::GetPin(const ConnectionInfo &pinInfo)
{
    Pin* pin=0;
    bool autoCreate=false;

    if(IsInError())
        autoCreate=true;

    foreach(PinsList *lst, pinLists) {
        if(lst->connInfo.type == pinInfo.type && lst->connInfo.direction == pinInfo.direction) {
            pin=lst->GetPin(pinInfo.pinNumber,autoCreate);
            if(pin)
                return pin;
        }
    }

    return 0;
}

/*!
  Create a small model used when the object is parked
  The caller is the owner of the return pointer and should delete it
  \return a pointer to the QStandardItem
  */
QStandardItem *Object::GetParkingItem()
{
    QStandardItem *modelNode=new QStandardItem();
    modelNode->setData(GetIndex(),UserRoles::value);
    modelNode->setData(objectName(), Qt::DisplayRole);

    //suspend the object if not used in the next program
    QTimer::singleShot(50,this,SLOT(SuspendIfParked()));

    return modelNode;
}

/*!
  Create a full model used when the object is on a panel
  The caller is the owner of the return pointer and should delete it
  \return a pointer to the QStandardItem
  */
QStandardItem *Object::GetFullItem()
{
    QStandardItem *modelNode = new QStandardItem();
    modelNode->setData(QVariant::fromValue(objInfo), UserRoles::objInfo);
    modelNode->setData(GetIndex(), UserRoles::value);
    modelNode->setData(objectName(), Qt::DisplayRole);
    modelNode->setData(errorMessage, UserRoles::errorMessage);
    Resume();
    return modelNode;
}

/*!
  Set the container Id, called by the parent container
  notify the children pins
  \param id the new container id
  */
void Object::SetContainerId(quint16 id)
{
    containerId = id;

    foreach(PinsList *lst, pinLists) {
        lst->SetContainerId(containerId);
    }
}

/*!
  Update the view
  */
//QStandardItem * Object::UpdateModelNode()
//{
//    if(!modelIndex.isValid())
//        return 0;

//    QStandardItem *modelNode = myHost->GetModel()->itemFromIndex(modelIndex);
//    if(!modelNode) {
//        LOG("node not found"<<modelIndex);
//        return 0;
//    }

//    modelNode->setData(QVariant::fromValue(objInfo), UserRoles::objInfo);
//    modelNode->setData(errorMessage, UserRoles::errorMessage);

//    foreach(PinsList *lst, pinLists) {
//        lst->UpdateModelNode(modelNode);
//    }

//    return modelNode;
//}

/*!
  Called when a parameter pin has changed
  \param pinInfo the modified pin
  \param value the new value
  */
void Object::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    //editor pin
    if(pinInfo.pinNumber==FixedPinNumber::editorVisible) {
        int val = static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(FixedPinNumber::editorVisible))->GetStepIndex();
        if(val)
            QTimer::singleShot(0, this, SLOT(OnShowEditor()));
        else
            QTimer::singleShot(0, this, SLOT(OnHideEditor()));
    }
}

/*!
  Toggle the editor (if the object has one) by changing the editor pin value
  \param visible true to show, false to hide
  */
bool Object::ToggleEditor(bool visible)
{
    if(!listParameterPinIn)
        return false;

    ParameterPin *pin = static_cast<ParameterPin*>(listParameterPinIn->listPins.value(FixedPinNumber::editorVisible,0));
    if(!pin)
        return false;
    if(static_cast<bool>(pin->GetStepIndex())==visible)
        return false;
    pin->ChangeValue(visible);
    return true;
}

/*!
  Get the current learning state
  \return LearningMode::Enum can be learn, unlearn or off
  */
LearningMode::Enum Object::GetLearningMode()
{
    if(!listParameterPinIn)
        return LearningMode::off;

    if(!listParameterPinIn->listPins.contains(FixedPinNumber::learningMode))
        return LearningMode::off;

    return (LearningMode::Enum)static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(FixedPinNumber::learningMode))->GetStepIndex();
}

/*!
  Set the object view status (position, size, ...) defined by the container
  \param[in] attr an ObjectContainerAttribs
  */
void Object::SetContainerAttribs(const ObjectContainerAttribs &attr)
{
//    if(!modelIndex.isValid())
//        return;

//    QStandardItem *item = myHost->GetModel()->itemFromIndex(modelIndex);
//    if(!item)
//        return;

//    item->setData(attr.position, UserRoles::position);
//    item->setData(attr.editorVisible, UserRoles::editorVisible);
//    item->setData(attr.editorPosition, UserRoles::editorPos);
//    item->setData(attr.editorSize, UserRoles::editorSize);
//    item->setData(attr.editorHScroll, UserRoles::editorHScroll);
//    item->setData(attr.editorVScroll, UserRoles::editorVScroll);
}

/*!
  Get the object view status, the status is saved by the container in a ContainerProgram
  \param[out] attr an ObjectContainerAttribs containing the object status
  */
void Object::GetContainerAttribs(ObjectContainerAttribs &attr)
{
//    if(!modelIndex.isValid())
//        return;

//    attr.position = modelIndex.data(UserRoles::position).toPointF();
//    attr.editorVisible = modelIndex.data(UserRoles::editorVisible).toBool();
//    attr.editorPosition = modelIndex.data(UserRoles::editorPos).toPoint();
//    attr.editorSize = modelIndex.data(UserRoles::editorSize).toSize();
//    attr.editorHScroll = modelIndex.data(UserRoles::editorHScroll).toInt();
//    attr.editorVScroll = modelIndex.data(UserRoles::editorVScroll).toInt();
}

/*!
  Copy the position, editor visibility and learning state, used by HostModel when a plugin is replaced
  \param objPtr destination object
  */
void Object::CopyStatusTo(QSharedPointer<Object>objPtr)
{
    ObjectContainerAttribs attr;
    GetContainerAttribs(attr);
    objPtr->SetContainerAttribs(attr);

    if(listParameterPinIn->listPins.contains(FixedPinNumber::editorVisible)) {
        Connectables::ParameterPinIn* oldEditPin = static_cast<Connectables::ParameterPinIn*>(listParameterPinIn->GetPin(FixedPinNumber::editorVisible));
        Connectables::ParameterPinIn* newEditPin = static_cast<Connectables::ParameterPinIn*>(objPtr->listParameterPinIn->GetPin(FixedPinNumber::editorVisible));
        if(newEditPin && oldEditPin)
            newEditPin->ChangeValue(oldEditPin->GetValue());
    }

    if(listParameterPinIn->listPins.contains(FixedPinNumber::learningMode)) {
        Connectables::ParameterPinIn* oldLearnPin = static_cast<Connectables::ParameterPinIn*>(listParameterPinIn->GetPin(FixedPinNumber::learningMode));
        Connectables::ParameterPinIn* newLearnPin = static_cast<Connectables::ParameterPinIn*>(objPtr->listParameterPinIn->GetPin(FixedPinNumber::learningMode));
        if(newLearnPin && oldLearnPin)
            newLearnPin->ChangeValue(oldLearnPin->GetValue());
    }
}

void Object::SetBufferSize(unsigned long size)
{
    foreach(Pin *pin, listAudioPinIn->listPins) {
        static_cast<AudioPin*>(pin)->SetBufferSize(size);
    }
    foreach(Pin *pin, listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->SetBufferSize(size);
    }
}

void Object::UserRemovePin(const ConnectionInfo &info)
{
    if(info.type!=PinType::Parameter)
        return;

    if(!info.isRemoveable)
        return;

    switch(info.direction) {
        case PinDirection::Input :
            listParameterPinIn->AsyncRemovePin(info.pinNumber);
            OnProgramDirty();
            break;
        case PinDirection::Output :
            listParameterPinOut->AsyncRemovePin(info.pinNumber);
            OnProgramDirty();
            break;
    }
}

void Object::UserAddPin(const ConnectionInfo &info)
{
    if(info.type!=PinType::Parameter)
        return;

    switch(info.direction) {
        case PinDirection::Input :
            listParameterPinIn->AsyncAddPin(info.pinNumber);
            OnProgramDirty();
            break;
        case PinDirection::Output :
            listParameterPinOut->AsyncAddPin(info.pinNumber);
            OnProgramDirty();
            break;
    }
}

/*!
  Called by PinsList to create a pin
  \param info ConnectionInfo defining the pin to be created
  \return a pointer to the new pin, 0 if not created
  */
Pin* Object::CreatePin(const ConnectionInfo &info)
{
    switch(info.direction) {
        case PinDirection::Input :
            switch(info.type) {
                case PinType::Audio : {
                    return new AudioPin(this,info.direction,info.pinNumber,myHost->GetBufferSize(),doublePrecision);
                }

                case PinType::Midi : {
                    return new MidiPinIn(this,info.pinNumber);
                }

                case PinType::Bridge : {
                    return new BridgePinIn(this,info.pinNumber,info.bridge);
                }

                default :
                    return 0;
            }
            break;

        case PinDirection::Output :
            switch(info.type) {
                case PinType::Audio : {
                    return new AudioPin(this,info.direction,info.pinNumber,myHost->GetBufferSize(),doublePrecision);
                }

                case PinType::Midi : {
                    return new MidiPinOut(this,info.pinNumber);
                }

                case PinType::Bridge : {
                    return new BridgePinOut(this,info.pinNumber,info.bridge);
                }

                default :
                    return 0;
            }
            break;

        default :
            return 0;
    }

    return 0;
}

/*!
  Put the object in a stream
  \param[in] out a QDataStream
  \return the stream
  */
QDataStream & Object::toStream(QDataStream & out) const
{
    out << (qint16)GetIndex();
    out << sleep;
    out << listenProgramChanges;

    out << (quint16)listPrograms.size();
    hashPrograms::const_iterator i = listPrograms.constBegin();
    while(i!=listPrograms.constEnd()) {
        out << (quint16)i.key();
        out << *i.value();
        ++i;
    }

    out << (quint16)currentProgId;
    return out;
}

/*!
  Load the object from a stream
  \param[in] in a QDataStream
  \return the stream
  */
bool Object::fromStream(QDataStream & in)
{
    qint16 id;
    in >> id;
    savedIndex=id;
    in >> sleep;
    in >> listenProgramChanges;

    quint16 nbProg;
    in >> nbProg;
    for(quint16 i=0; i<nbProg; i++) {
        quint16 progId;
        in >> progId;

        ObjectProgram *prog = new ObjectProgram();
        in >> *prog;
        if(listPrograms.contains(progId)) {
            if(progId==currentProgId) {
                currentProgram=prog;
            }
            delete listPrograms.take(progId);
        }
        listPrograms.insert(progId,prog);
    }

    quint16 savedProgId;
    in >> savedProgId;

    if(in.status()!=QDataStream::Ok) {
        LOG("err"<<in.status());
        return false;
    }

    return true;
}

void Object::ProgramToStream (int progId, QDataStream &out)
{
    bool dirty = IsDirty();
    ObjectProgram *prog = 0;

    if(progId == currentProgId) {
        prog = new ObjectProgram(*currentProgram);
        prog->Save(listParameterPinIn,listParameterPinOut);
    } else {
        prog = listPrograms.value(progId,0);
    }

    if(!prog) {
        out << (quint8)0;
        return;
    }
    out << (quint8)1;

    out << (quint8)dirty;

    out << *prog;

    if(progId == currentProgId)
        delete prog;
}

void Object::ProgramFromStream (int progId, QDataStream &in)
{
    quint8 valid=0;
    in >> valid;
    if(valid!=1)
        return;

    quint8 dirty;
    in >> dirty;

    if(progId == currentProgId) {
        UnloadProgram();
        currentProgram = new ObjectProgram();
        in >> *currentProgram;
        if(dirty)
            currentProgram->SetDirty();
    } else {
        ObjectProgram *prog = new ObjectProgram();
        in >> *prog;

        if(listPrograms.contains(progId))
            delete listPrograms.take(progId);
        listPrograms.insert(progId,prog);
    }
}

/*!
  overload stream out
  */
QDataStream & operator<< (QDataStream & out, const Connectables::Object& value)
{
    return value.toStream(out);
}

/*!
  overload stream in
  */
//QDataStream & operator>> (QDataStream & in, Connectables::Object& value)
//{
//    return value.fromStream(in);
//}

void Object::GetInfos(MsgObject &msg)
{
    msg.prop[MsgObject::Id] = GetIndex();

    msg.prop[MsgObject::Add]=info().nodeType;
    msg.prop[MsgObject::Type]=info().objType;
    msg.prop[MsgObject::Name]=info().name;

    if(!errorMessage.isEmpty()) {
        msg.prop[MsgObject::Message]=errorMessage;
    }

    QMap<QString, PinsList*>::iterator i = pinLists.constBegin();
    while(i!=pinLists.constEnd()) {
        PinsList *lst = i.value();
        if(!lst->IsVisible()) {
            ++i;
            continue;
        }
        MsgObject lstPinMsg(GetIndex());
        lst->GetInfos(lstPinMsg);
        msg.children << lstPinMsg;
        ++i;
    }
}

void Object::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Remove)) {
        QSharedPointer<Connectables::Object> objPtr = myHost->objFactory->GetObjectFromId( GetIndex() );
        if(!objPtr) {
            LOG("obj not found")
            return;
        }

        int removeType = msg.prop[MsgObject::Type].toInt();
        myHost->undoStack.push( new ComRemoveObject(myHost, objPtr, static_cast<RemoveType::Enum>(removeType) ) );
        return;
    }

    if(msg.prop.contains(MsgObject::ObjectsToLoad)) {

        int insertType = msg.prop[MsgObject::Type].toInt();
        QSharedPointer<Object> targetObj = myHost->objFactory->GetObjectFromId( GetIndex() );
        if(!targetObj) {
            LOG("obj not found")
            return;
        }
        QSharedPointer<Container> targetContainer = myHost->objFactory->GetObjectFromId( containerId ).staticCast<Container>();;
        if(!targetContainer) {
            LOG("obj not found")
            return;
        }

        QDataStream streamObj(&msg.prop[MsgObject::ObjectsToLoad].toByteArray(), QIODevice::ReadOnly);
        while(!streamObj.atEnd()) {
            ObjectInfo newInfo;
            newInfo.fromStream(streamObj);
            ComAddObject *com = new ComAddObject(myHost, newInfo, targetContainer, targetObj, static_cast<InsertionType::Enum>(insertType) );
            myHost->undoStack.push( com );
        }

        return;
    }

    if(msg.prop.contains(MsgObject::FilesToLoad)) {
        QStringList lstFiles = msg.prop[MsgObject::FilesToLoad].toStringList();
        foreach(const QString filename, lstFiles) {
            QFileInfo info;
            info.setFile( filename );
            if ( info.isFile() && info.isReadable() ) {
                QString fileType(info.suffix().toLower());

                //fxb file
                if( fileType == VST_BANK_FILE_EXTENSION || fileType == VST_PROGRAM_FILE_EXTENSION) {
                    continue;
                }
            }
        }
        return;
    }
}

void Object::SetMsgEnabled(bool enab)
{
    MsgHandler::SetMsgEnabled(enab);
    foreach(PinsList *lst, pinLists) {
        lst->SetMsgEnabled(enab);
    }

    //we're enabled, parent is not : send update
    if(enab && !msgCtrl->listObj[containerId]->MsgEnabled()) {
        MsgObject msg(containerId);
        GetInfos(msg);
        msgCtrl->SendMsg(msg);
    }
}
