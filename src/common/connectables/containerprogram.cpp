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


#include "containerprogram.h"
#include "mainhost.h"
#include "container.h"
#include "object.h"
//#include "renderer/renderernode.h"

using namespace Connectables;

QTime ContainerProgram::unsavedTime;

ContainerProgram::ContainerProgram(MainHost *myHost,Container *container) :
    lastModificationTime(unsavedTime),
    savedTime(unsavedTime),
    container(container),
    dirty(false),
    myHost(myHost),
    collectedListOfAddedCables(0),
    collectedListOfRemovedCables(0),
    listAddedCablesIds(0),
    listRemovedCablesIds(0)
{
}

ContainerProgram::ContainerProgram(const ContainerProgram& c) :
    lastModificationTime(c.lastModificationTime),
    savedTime(c.savedTime),
    container(c.container),
    dirty(false),
    myHost(c.myHost),
    collectedListOfAddedCables(0),
    collectedListOfRemovedCables(0),
    listAddedCablesIds(0),
    listRemovedCablesIds(0)
{
    foreach(QSharedPointer<Object> objPtr, c.listObjects) {
        listObjects << objPtr;
    }

    foreach(QSharedPointer<Cable>cab, c.listCables) {
        listCables << cab;
    }

    QMap<int,ObjectContainerAttribs>::ConstIterator i = c.mapObjAttribs.constBegin();
    while(i!=c.mapObjAttribs.constEnd()) {
        mapObjAttribs.insert(i.key(),i.value());
        ++i;
    }

    savedRenderMap=c.savedRenderMap;
}

ContainerProgram::~ContainerProgram()
{
//    foreach(QSharedPointer<Cable>c, listCables) {
//        delete c;
//    }
    listCables.clear();

    listObjects.clear();
    mapObjAttribs.clear();

//    foreach(RendererNode *node, listOfRendererNodes) {
//        delete node;
//    }
}

void ContainerProgram::Remove(int prgId)
{
    foreach(QSharedPointer<Object> objPtr, listObjects) {
        objPtr->RemoveProgram(prgId);
    }
}

bool ContainerProgram::PinExistAndVisible(const ConnectionInfo &info)
{
    if(info.type==PinType::Bridge)
        return true;

    Pin* pin=myHost->objFactory->GetPin(info);
    if(!pin)
        return false;
    if(!pin->GetVisible())
        return false;
    return true;
}

void ContainerProgram::AddToCableList(hashCables *l)
{
    foreach(QSharedPointer<Cable>c, listCables) {
        l->insert(c->GetInfoOut(), c);
    }
}

void ContainerProgram::Load(int progId)
{
    foreach(QSharedPointer<Object> objPtr, listObjects) {
        if(objPtr) {
            //the container adds the object if not already there
//            container->AddChildObject(objPtr);
            objPtr->LoadProgram(progId);
        }
    }

    foreach(QSharedPointer<Cable>cab, listCables) {

        //check if the pin exists,
        //if the object is in error mode, dummy pins will be created
        if(PinExistAndVisible(cab->GetInfoOut()) &&
           PinExistAndVisible(cab->GetInfoIn())) {
            Connectables::Pin *pin = myHost->objFactory->GetPin(cab->GetInfoOut());
            if(pin)
                pin->AddCable(cab);

//            cab->AddToParentNode(container->GetCablesIndex());
        } else {
            //delete cable from program if pins are not found and can't be created
            listCables.removeAll(cab);
        }
    }

    QMap<int,ObjectContainerAttribs>::Iterator i = mapObjAttribs.begin();
    while(i!=mapObjAttribs.end()) {
        QSharedPointer<Object> obj = myHost->objFactory->GetObjectFromId(i.key());
        if(obj) {
            obj->SetContainerAttribs(i.value());
        } else {
            //delete attrib if object not found
            i=mapObjAttribs.erase(i);
        }
        ++i;
    }

    ResetDirty();
}

void ContainerProgram::Unload()
{
    foreach(QSharedPointer<Cable>cab, listCables) {
        Connectables::Pin *pin = myHost->objFactory->GetPin(cab->GetInfoOut());
        if(pin)
            pin->RemoveCable(cab);
//        cab->RemoveFromParentNode(container->GetCablesIndex());
    }
    foreach(QSharedPointer<Object> obj, listObjects) {
        if(obj)
            obj->UnloadProgram();
    }
}

void ContainerProgram::GetInfos(MsgObject &msg)
{
    foreach(QSharedPointer<Object> obj, listObjects) {
        if(obj) {
//            MsgObject a(container->GetIndex());
            _MSGOBJ(a,container->GetIndex());
            obj->GetInfos(a);
            msg.children << a;

            //add view attribs (position, size, etc.)
            if(mapObjAttribs.contains(obj->GetIndex())) {
//                MsgObject attr(obj->GetIndex());
                _MSGOBJ(attr,obj->GetIndex());
                attr.prop[MsgObject::State] = QVariant::fromValue( mapObjAttribs[obj->GetIndex()] );
                msg.children << attr;

            }

        }
    }

    foreach(QSharedPointer<Cable>cab, listCables) {
        if(cab) {
//            MsgObject a(container->GetIndex());
            _MSGOBJ(a,container->GetIndex());
            cab->GetInfos(a);
            msg.children << a;
        }
    }

    QMap<int,ObjectContainerAttribs>::Iterator i = mapObjAttribs.begin();
    while(i!=mapObjAttribs.end()) {
        QSharedPointer<Object> obj = myHost->objFactory->GetObjectFromId(i.key());
        if(!obj) {
            //delete attrib if object not found
            i=mapObjAttribs.erase(i);
        }
        ++i;
    }
}

void ContainerProgram::SetMsgEnabled(bool enab)
{
    foreach(QSharedPointer<Object> obj, listObjects) {
        if(obj) {
            obj->SetMsgEnabled(enab);
        }
    }

    foreach(QSharedPointer<Cable>cab, listCables) {
        if(cab) {
            cab->SetMsgEnabled(enab);
        }
    }
}

void ContainerProgram::SaveRendererState()
{
    const QTime t = container->GetLastModificationTime();
    if(!savedTime.isValid() || t > savedTime) {
        savedTime = QTime::currentTime();
        lastModificationTime = savedTime;

        //savedRenderMap.clear();
        myHost->GetRenderMap(savedRenderMap);

//        qDeleteAll(listOfRendererNodes);
//        listOfRendererNodes.clear();
//        listOfRendererNodes = myHost->GetRenderer()->SaveNodes();
    }
}

void ContainerProgram::LoadRendererState()
{

    //if an object contains an initialDelay we need to update everything
    if(myHost->objFactory->listDelayObj.isEmpty()) {
        myHost->ResetDelays();
        const QTime t = container->GetLastModificationTime();
        if(t > lastModificationTime || lastModificationTime==unsavedTime) {
            //my renderer map is outdated
            myHost->SetSolverUpdateNeeded();
        } else {
            myHost->SetRenderMap(savedRenderMap);
//            myHost->GetRenderer()->LoadNodes( listOfRendererNodes );
        }
    } else {
        LOG("update everything for delays");
        myHost->SetSolverUpdateNeeded();
    }
}

void ContainerProgram::ResetDelays()
{
    foreach(QSharedPointer<Cable>c, listCables) {
        if(c)
            c->SetDelay(0);
    }
}

void ContainerProgram::ParkAllObj()
{
    foreach(QSharedPointer<Object> obj, listObjects) {
        container->ParkChildObject(obj);
    }
}

bool ContainerProgram::IsDirty()
{
    if(dirty)
        return true;

    foreach(QSharedPointer<Object> obj, listObjects) {
        if(obj) {
            if(obj->IsDirty())
                return true;

            ObjectContainerAttribs attr;
            obj->GetContainerAttribs(attr);
            if(attr != mapObjAttribs.value(obj->GetIndex()))
                return true;
        }
    }
    return false;
}

void ContainerProgram::Save(bool saveChildPrograms)
{

    if(saveChildPrograms) {
        foreach(QSharedPointer<Object> objPtr, listObjects) {
            objPtr->SaveProgram();
        }
    }

    mapObjAttribs.clear();
    foreach(QSharedPointer<Object> obj, listObjects) {
        if(obj) {
            ObjectContainerAttribs attr;
            obj->GetContainerAttribs(attr);
            mapObjAttribs.insert(obj->GetIndex(),attr);
        }
    }

    foreach(QSharedPointer<Object> obj, container->listStaticObjects) {
        if(obj) {
            //don't save bridges position
            if(obj->info().nodeType==NodeType::bridge) {
                continue;
            }

            ObjectContainerAttribs attr;
            obj->GetContainerAttribs(attr);
            mapObjAttribs.insert(obj->GetIndex(),attr);
        }
    }
    ResetDirty();
}

void ContainerProgram::AddObject(QSharedPointer<Object> objPtr)
{
    listObjects << objPtr;
    container->AddChildObject(objPtr);
    objPtr->SetContainerAttribs( ObjectContainerAttribs() );
    SetDirty();
}

void ContainerProgram::RemoveObject(QSharedPointer<Object> objPtr)
{
    RemoveCableFromObj(objPtr->GetIndex());
    listObjects.removeAll(objPtr);
    container->ParkChildObject(objPtr);
    SetDirty();
}

void ContainerProgram::ReplaceObject(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> replacedObjPtr)
{
    //AddObject(newObjPtr);
    CopyCablesFromObj( newObjPtr->GetIndex(), replacedObjPtr->GetIndex() );
    //RemoveObject(replacedObjPtr);
}

//void ContainerProgram::SendMsg(const ConnectionInfo &senderPin,const PinMessage::Enum msgType,void *data)
//{
////    QMutexLocker lock(mutexListCables);

//    foreach(Cable *c, listCables) {
//        if(c->GetInfoOut()==senderPin)
//            c->Render(msgType,data);
//    }
//}

bool ContainerProgram::AddCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin, bool hidden)
{
    if(CableExists(outputPin,inputPin))
        return true;

    if(!outputPin.CanConnectTo(inputPin))
        return false;

    //check if the pin exists,
    //if the object is in error mode, dummy pins will be created
    if(!PinExistAndVisible(inputPin) || !PinExistAndVisible(outputPin))
        return false;

    QSharedPointer<Cable>cab(new Cable(myHost,outputPin,inputPin));
    listCables << cab;
    if(container->MsgEnabled())
        cab->SetMsgEnabled(true);

    if(collectedListOfAddedCables)
        *collectedListOfAddedCables << QPair<ConnectionInfo,ConnectionInfo>(outputPin,inputPin);
    if(listAddedCablesIds)
        *listAddedCablesIds << cab;

//    if(!hidden && container) {
//        cab->AddToParentNode(container->GetCablesIndex());
//    }

    Connectables::Pin *pin = myHost->objFactory->GetPin(outputPin);
    if(pin)
        pin->AddCable(cab);

    SetDirty();
    return true;
}

void ContainerProgram::RemoveCable(QSharedPointer<Cable>cab)
{
    Connectables::Pin *pin = myHost->objFactory->GetPin(cab->GetInfoOut());
    if(pin)
        pin->RemoveCable(cab);

    if(collectedListOfRemovedCables)
        *collectedListOfRemovedCables << QPair<ConnectionInfo,ConnectionInfo>(cab->GetInfoOut(),cab->GetInfoIn());
    if(listRemovedCablesIds)
        *listRemovedCablesIds << cab->GetIndex();

//    cab->RemoveFromParentNode(container->GetCablesIndex());
    SetDirty();
    listCables.removeAll(cab);
    //    delete cab;
}

void ContainerProgram::RemoveCable(const QModelIndex & index)
{
    ConnectionInfo outInfo = index.data(UserRoles::connectionInfo).value<ConnectionInfo>();
    ConnectionInfo inInfo = index.data(UserRoles::connectionInfo).value<ConnectionInfo>();
    RemoveCable(outInfo,inInfo);
}

void ContainerProgram::RemoveCable(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin)
{
    int i=listCables.size()-1;
    while(i>=0) {
        QSharedPointer<Cable>cab = listCables.at(i);
        if(cab->GetInfoOut()==outputPin && cab->GetInfoIn()==inputPin) {
            RemoveCable(cab);
            return;
        }
        --i;
    }
}

void ContainerProgram::RemoveCableFromPin(const ConnectionInfo &pin)
{
    int i=listCables.size()-1;
    while(i>=0) {
        QSharedPointer<Cable>cab = listCables.at(i);
        if(cab->GetInfoOut()==pin || cab->GetInfoIn()==pin) {
            RemoveCable(cab);
        }
        --i;
    }
}

void ContainerProgram::RemoveCableFromObj(int objId)
{
    int i=listCables.size()-1;
    while(i>=0) {
        QSharedPointer<Cable>cab = listCables.at(i);
        if(cab->GetInfoOut().objId==objId || cab->GetInfoIn().objId==objId ||
           cab->GetInfoOut().container==objId || cab->GetInfoIn().container==objId) {
            RemoveCable(cab);
        }
        --i;
    }
}

void ContainerProgram::CreateBridgeOverObj(int objId)
{
    int i=listCables.size()-1;
    while(i>=0) {
        QSharedPointer<Cable>cab = listCables.at(i);
        if(cab->GetInfoOut().objId==objId || cab->GetInfoIn().objId==objId ||
           cab->GetInfoOut().container==objId || cab->GetInfoIn().container==objId) {

            //for all output cables
            if(cab->GetInfoOut().objId==objId && cab->GetInfoOut().type!=PinType::Parameter ) {
                int j=listCables.size()-1;
                while(j>=0) {
                    QSharedPointer<Cable>otherCab = listCables.at(j);
                    ConnectionInfo otherPin( cab->GetInfoOut() );
                    otherPin.direction=PinDirection::Input;

                    //find corresponding input cables
                    if(otherCab->GetInfoIn()==otherPin) {
                        //create a bridge
                        AddCable(otherCab->GetInfoOut(), cab->GetInfoIn());
                    }
                    --j;
                }
            }

            //for all input cables
            if(cab->GetInfoIn().objId==objId && cab->GetInfoIn().type!=PinType::Parameter ) {
                int j=listCables.size()-1;
                while(j>=0) {
                    QSharedPointer<Cable>otherCab = listCables.at(j);
                    ConnectionInfo otherPin( cab->GetInfoOut() );
                    otherPin.direction=PinDirection::Output;

                    //find corresponding output cables
                    if(otherCab->GetInfoOut()==otherPin) {
                        //create a bridge
                        AddCable(cab->GetInfoOut(), otherCab->GetInfoIn() );
                    }
                    --j;
                }
            }
        }
        --i;
    }
}

void ContainerProgram::CopyCablesFromObj(int newObjId, int oldObjId)
{
    int i=listCables.size()-1;
    while(i>=0) {
        QSharedPointer<Cable>cab = listCables.at(i);
        if(cab->GetInfoOut().objId==oldObjId) {
            ConnectionInfo newConnect = cab->GetInfoOut();
            newConnect.objId = newObjId;
            AddCable(newConnect, cab->GetInfoIn());
        }
        if(cab->GetInfoIn().objId==oldObjId) {
            ConnectionInfo newConnect = cab->GetInfoIn();
            newConnect.objId = newObjId;
            AddCable(cab->GetInfoOut(), newConnect);
        }
        --i;
    }
}

void ContainerProgram::MoveOutputCablesFromObj(int newObjId, int oldObjId)
{
    int i=listCables.size()-1;
    while(i>=0) {
        QSharedPointer<Cable>cab = listCables.at(i);
        if(cab->GetInfoOut().objId==oldObjId && cab->GetInfoOut().type!=PinType::Parameter) {
            ConnectionInfo newConnect = cab->GetInfoOut();
            newConnect.objId = newObjId;
            if( AddCable(newConnect, cab->GetInfoIn()) ) {
                RemoveCable(cab);
            }
        }
        --i;
    }
}

void ContainerProgram::MoveInputCablesFromObj(int newObjId, int oldObjId)
{
    int i=listCables.size()-1;
    while(i>=0) {
        QSharedPointer<Cable>cab = listCables.at(i);
        if(cab->GetInfoIn().objId==oldObjId && cab->GetInfoIn().type!=PinType::Parameter) {
            ConnectionInfo newConnect = cab->GetInfoIn();
            newConnect.objId = newObjId;
            if( AddCable(cab->GetInfoOut(), newConnect) ) {
                RemoveCable(cab);
            }
        }
        --i;
    }
}

void ContainerProgram::GetListOfConnectedPinsTo(const ConnectionInfo &pin, QList<ConnectionInfo> &list)
{
    int i=listCables.size()-1;
    while(i>=0) {
        QSharedPointer<Cable>cab = listCables.at(i);
        if(cab->GetInfoIn()==pin)
            list << cab->GetInfoOut();
        if(cab->GetInfoOut()==pin)
            list << cab->GetInfoIn();
        --i;
    }
}

bool ContainerProgram::CableExists(const ConnectionInfo &outputPin, const ConnectionInfo &inputPin)
{
    foreach(QSharedPointer<Cable>c, listCables) {
        if(c->GetInfoOut()==outputPin && c->GetInfoIn()==inputPin)
            return true;
    }
    return false;
}

ContainerProgram::ContainerProgram(MainHost *myHost, Container *container, QJsonObject &json, int &id) :
    ContainerProgram(myHost, container)
{
    id = json["id"].toInt();

    QJsonArray objArray = json["objectsIds"].toArray();
    for (int i = 0; i < objArray.size(); ++i) {
        int savedId = objArray[i].toInt();
        int newid = myHost->objFactory->IdFromSavedId(savedId);
        if(newid!=-1) {
            listObjects << myHost->objFactory->GetObjectFromId(newid);
        }
    }

    QJsonArray cableArray = json["cables"].toArray();
    for (int i = 0; i < cableArray.size(); ++i) {
        QJsonObject jCable = cableArray[i].toObject();
        Cable *cab = new Cable(myHost, jCable);
        listCables << QSharedPointer<Cable>(cab);
    }

    QJsonArray attrArray = json["attrs"].toArray();
    for (int i = 0; i < attrArray.size(); ++i) {
        QJsonObject jAttr = attrArray[i].toObject();
        int savedId;
        ObjectContainerAttribs attr(jAttr,savedId);
        int objId=myHost->objFactory->IdFromSavedId(savedId);
        if(objId!=-1) {
            mapObjAttribs.insert(objId,attr);
        }
    }

    ResetDirty();
}

void ContainerProgram::toJson(QJsonObject &json, int id) const
{
    json["id"] = id;

    QJsonArray objArray;
    foreach(QSharedPointer<Object> objPtr, listObjects) {
        objArray.append( objPtr->GetIndex() );
    }
    json["objectsIds"] = objArray;

    QJsonArray cableArray;
    foreach(QSharedPointer<Cable>cab, listCables) {
        QJsonObject jCable;
        cab->toJson(jCable);
        cableArray.append(jCable);
    }
    json["cables"] = cableArray;

    QJsonArray attrArray;
    QMap<int,ObjectContainerAttribs>::ConstIterator i = mapObjAttribs.constBegin();
    while(i!=mapObjAttribs.constEnd()) {
        QJsonObject jAttr;
        i.value().toJson(jAttr, i.key());
        attrArray.append(jAttr);
        ++i;
    }
    json["attrs"] = attrArray;
}

QDataStream & ContainerProgram::toStream (QDataStream& out) const
{
    qint32 nbObj = listObjects.size();
    out << nbObj;
    foreach(QSharedPointer<Object> objPtr, listObjects) {
        out << (qint32)objPtr->GetIndex();
    }

    out << (quint16)listCables.size();
    foreach(QSharedPointer<Cable>cab, listCables) {
        out << cab->GetInfoOut();
        out << cab->GetInfoIn();
        out << (quint32)cab->GetDelay();
    }

    out << (qint32)mapObjAttribs.size();
    QMap<qint32,ObjectContainerAttribs>::ConstIterator i = mapObjAttribs.constBegin();
    while(i!=mapObjAttribs.constEnd()) {
        out << i.key();
        out << i.value();
        ++i;
    }

    return out;
}

QDataStream & ContainerProgram::fromStream (QDataStream& in)
{
    qint32 nbobj;
    in >> nbobj;
    for(qint32 i=0; i<nbobj; i++) {
        qint32 id;
        in >> id;
        qint32 newid = myHost->objFactory->IdFromSavedId(id);
        if(newid==-1)
            return in;
        listObjects << myHost->objFactory->GetObjectFromId(newid);
    }

    quint16 nbCables;
    in >> nbCables;
    for(quint16 i=0; i<nbCables; i++) {
        ConnectionInfo infoOut;
        infoOut.myHost = myHost;
        in >> infoOut;

        ConnectionInfo infoIn;
        infoIn.myHost = myHost;
        in >> infoIn;

        //check if this pin exists
        //myHost->objFactory->GetPin(infoOut);
        //myHost->objFactory->GetPin(infoIn);

        quint32 d=0;
        if(myHost->currentFileVersion>=17)
            in >> d;

        Cable *cab = new Cable(myHost,infoOut,infoIn);
        cab->SetDelay(d);
        listCables << QSharedPointer<Cable>(cab);
    }

    qint32 nbPos;
    in >> nbPos;
    for(qint32 i=0; i<nbPos; i++) {
        qint32 objId;
        ObjectContainerAttribs attr;
        in >> objId;
        in >> attr;
        objId=myHost->objFactory->IdFromSavedId(objId);
        mapObjAttribs.insert(objId,attr);
    }

    ResetDirty();
    return in;
}

QDataStream & operator<< (QDataStream & out, const Connectables::ContainerProgram& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, Connectables::ContainerProgram& value)
{
    return value.fromStream(in);
}


