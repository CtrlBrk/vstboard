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

#include "precomp.h"
#include "containerprogram.h"
#include "mainhost.h"
#include "objects/container.h"
#include "objects/object.h"
#include "renderer/renderernode.h"

using namespace Connectables;

QTime ContainerProgram::unsavedTime;

ContainerProgram::ContainerProgram(MainHost *myHost,Container *container) :
    container(container),
    dirty(false),
    myHost(myHost),
    collectedListOfAddedCables(0),
    collectedListOfRemovedCables(0)
{
}

ContainerProgram::ContainerProgram(const ContainerProgram& c) :
    container(c.container),
    dirty(false),
    myHost(c.myHost),
    collectedListOfAddedCables(0),
    collectedListOfRemovedCables(0)
{
    foreach(QSharedPointer<Object> objPtr, c.listObjects) {
        listObjects << objPtr;
    }

    foreach(Cable* cab, c.listCables) {
        listCables << new Cable(*cab);
    }

    QMap<int,ObjectContainerAttribs>::ConstIterator i = c.mapObjAttribs.constBegin();
    while(i!=c.mapObjAttribs.constEnd()) {
        mapObjAttribs.insert(i.key(),i.value());
        ++i;
    }

    foreach(RendererNode *node, c.listOfRendererNodes) {
        listOfRendererNodes << new RendererNode(*node);
    }
    lastModificationTime = c.lastModificationTime;
}

ContainerProgram::~ContainerProgram()
{
    foreach(Cable *c, listCables) {
        delete c;
    }
    listCables.clear();

    listObjects.clear();
    mapObjAttribs.clear();

    foreach(RendererNode *node, listOfRendererNodes) {
        delete node;
    }
}

void ContainerProgram::Remove(int prgId)
{
    foreach(QSharedPointer<Object> objPtr, listObjects) {
        objPtr->RemoveProgram(prgId);
    }
}

bool ContainerProgram::PinExistAndVisible(const MetaInfo &info)
{
    if(info.Meta(MetaInfos::Media).toInt()==MediaTypes::Bridge)
        return true;

    Pin* pin=myHost->objFactory->GetPin(info);
    if(!pin)
        return false;
    if(info.Meta(MetaInfos::Hidden).toBool())
        return false;
    return true;
}

void ContainerProgram::Load(int progId)
{
    foreach(QSharedPointer<Object> objPtr, listObjects) {
        if(objPtr) {
//            container->AddChildObject(objPtr);
            objPtr->LoadProgram(progId);
        }
    }

    foreach(Cable *cab, listCables) {

        //check if the pin exists,
        //if the object is in error mode, dummy pins will be created
        if(PinExistAndVisible(cab->GetInfoOut()) &&
           PinExistAndVisible(cab->GetInfoIn())) {
//            cab->AddToParentNode(container->GetCablesIndex());
            cab->AddToView();
            myHost->OnCableAdded(cab);
        } else {
            //delete cable from program if pins are not found and can't be created
            listCables.removeAll(cab);
        }
    }

    QMap<int,ObjectContainerAttribs>::Iterator i = mapObjAttribs.begin();
    while(i!=mapObjAttribs.end()) {
        QSharedPointer<Object> obj = myHost->objFactory->GetObjectFromId(i.key());
        if(!obj.isNull()) {
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
    foreach(Cable *cab, listCables) {
        myHost->OnCableRemoved(cab);
//        cab->RemoveFromParentNode(container->GetCablesIndex());
        cab->RemoveFromView();
    }

    foreach(QSharedPointer<Object> obj, listObjects) {
        if(!obj.isNull())
            obj->UnloadProgram();
    }
}

void ContainerProgram::SaveRendererState()
{
    const QTime t = container->GetLastModificationTime();
    if(!savedTime.isValid() || t > savedTime) {
        savedTime = QTime::currentTime();
        lastModificationTime = savedTime;
        qDeleteAll(listOfRendererNodes);
        listOfRendererNodes.clear();
        listOfRendererNodes = myHost->GetRenderer()->SaveNodes();
    }
}

void ContainerProgram::LoadRendererState()
{
    const QTime t = container->GetLastModificationTime();
    if(t > lastModificationTime) {
        //my renderer map is outdated
        myHost->SetSolverUpdateNeeded();
    } else {
        myHost->GetRenderer()->LoadNodes( listOfRendererNodes );
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
        if(!obj.isNull()) {
            if(obj->IsDirty())
                return true;

            ObjectContainerAttribs attr;
            obj->GetContainerAttribs(attr);
            if(attr != mapObjAttribs.value( obj->ObjId() ))
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
        if(!obj.isNull()) {
            ObjectContainerAttribs attr;
            obj->GetContainerAttribs(attr);
            mapObjAttribs.insert(obj->ObjId(),attr);
        }
    }

    foreach(QSharedPointer<Object> obj, container->listStaticObjects) {
        if(!obj.isNull() ) {
            //don't save bridges position
            if(obj->Type()==MetaTypes::bridge) {
                continue;
            }

            ObjectContainerAttribs attr;
            obj->GetContainerAttribs(attr);
            mapObjAttribs.insert(obj->ObjId(),attr);
        }
    }
    ResetDirty();
}

void ContainerProgram::AddObject(QSharedPointer<Object> objPtr)
{
    listObjects << objPtr;
    container->AddChildObject(objPtr);
    SetDirty();
}

void ContainerProgram::RemoveObject(QSharedPointer<Object> objPtr)
{
    RemoveCableFromObj(objPtr->ObjId());
    listObjects.removeAll(objPtr);
    if(container->currentContainerProgram==this)
        container->ParkChildObject(objPtr);
    SetDirty();
}

void ContainerProgram::ReplaceObject(QSharedPointer<Object> newObjPtr, QSharedPointer<Object> replacedObjPtr)
{
    //AddObject(newObjPtr);
    CopyCablesFromObj( newObjPtr->ObjId(), replacedObjPtr->ObjId() );
    //RemoveObject(replacedObjPtr);
}

bool ContainerProgram::AddCable(const MetaInfo &outputPin, const MetaInfo &inputPin, bool hidden)
{
    if(CableExists(outputPin,inputPin))
        return true;

    if(!outputPin.CanConnectTo(inputPin))
        return false;

    //check if the pin exists,
    //if the object is in error mode, dummy pins will be created
    if(!PinExistAndVisible(inputPin) || !PinExistAndVisible(outputPin))
        return false;

    Cable *cab = new Cable(outputPin,inputPin);
    cab->SetObjId(MetaInfo::GetNextId());
    cab->SetContainer(container);
    cab->SetParent(container);
    listCables << cab;

    if(collectedListOfAddedCables)
        *collectedListOfAddedCables << QPair<MetaInfo,MetaInfo>(outputPin,inputPin);

    if(!hidden && container) {
//        cab->AddToParentNode(container->GetCablesIndex());
        cab->AddToView();
    }

    myHost->OnCableAdded(cab);
    SetDirty();
    return true;
}

void ContainerProgram::RemoveCable(Cable *cab)
{
    if(collectedListOfRemovedCables)
        *collectedListOfRemovedCables << QPair<MetaInfo,MetaInfo>(cab->GetInfoOut(),cab->GetInfoIn());

    listCables.removeAll(cab);
//    cab->RemoveFromParentNode(container->GetCablesIndex());
    cab->RemoveFromView();
    myHost->OnCableRemoved(cab);
    delete cab;
    SetDirty();
}

//void ContainerProgram::RemoveCable(const QModelIndex & index)
//{
//    RemoveCable(index.data(UserRoles::objInfo).value<ConnectionInfo>(),
//                index.data(UserRoles::objInfo).value<ConnectionInfo>());
//}

void ContainerProgram::RemoveCable(const MetaInfo &outputPin, const MetaInfo &inputPin)
{
    int i=listCables.size()-1;
    while(i>=0) {
        Cable *cab = listCables.at(i);
        if(cab->GetInfoOut().ObjId()==outputPin.ObjId() && cab->GetInfoIn().ObjId()==inputPin.ObjId()) {
            RemoveCable(cab);
            return;
        }
        --i;
    }
}

void ContainerProgram::RemoveCableFromPin(const MetaInfo &pin)
{
    int i=listCables.size()-1;
    while(i>=0) {
        Cable *cab = listCables.at(i);
        if(cab->GetInfoOut().ObjId()==pin.ObjId() || cab->GetInfoIn().ObjId()==pin.ObjId()) {
            RemoveCable(cab);
        }
        --i;
    }
}

void ContainerProgram::RemoveCableFromObj(int objId)
{
    int i=listCables.size()-1;
    while(i>=0) {
        Cable *cab = listCables.at(i);
        if(cab->GetInfoOut().ParentObjectId()==objId || cab->GetInfoIn().ParentObjectId()==objId ||
           cab->GetInfoOut().ContainerId()==objId || cab->GetInfoIn().ContainerId()==objId) {
            RemoveCable(cab);
        }
        --i;
    }
}

void ContainerProgram::CreateBridgeOverObj(int objId)
{
    int i=listCables.size()-1;
    while(i>=0) {
        Cable *cab = listCables.at(i);
        if(cab->GetInfoOut().ParentObjectId()==objId || cab->GetInfoIn().ParentObjectId()==objId ||
           cab->GetInfoOut().ContainerId()==objId || cab->GetInfoIn().ContainerId()==objId) {

            //for all output cables
            if(cab->GetInfoOut().ParentObjectId()==objId && cab->GetInfoOut().Meta(MetaInfos::Media).toInt()!=MediaTypes::Parameter ) {
                int j=listCables.size()-1;
                while(j>=0) {
                    Cable *otherCab = listCables.at(j);
                    MetaInfo otherPin( cab->GetInfoOut() );
                    otherPin.SetMeta(MetaInfos::Direction,Directions::Input);

                    if(myHost->objFactory->UpdatePinInfo(otherPin)) {
                        //find corresponding input cables
                        if(otherCab->GetInfoIn().ObjId()==otherPin.ObjId()) {
                            //create a bridge
                            AddCable(otherCab->GetInfoOut(), cab->GetInfoIn());
                        }
                    }
                    --j;
                }
            }

            //for all input cables
            if(cab->GetInfoIn().ParentObjectId()==objId && cab->GetInfoIn().Meta(MetaInfos::Media).toInt()!=MediaTypes::Parameter ) {
                int j=listCables.size()-1;
                while(j>=0) {
                    Cable *otherCab = listCables.at(j);
                    MetaInfo otherPin = cab->GetInfoIn();
                    otherPin.SetMeta(MetaInfos::Direction,Directions::Output);
                    myHost->objFactory->UpdatePinInfo(otherPin);

                    //find corresponding output cables
                    if(otherCab->GetInfoOut().ObjId()==otherPin.ObjId()) {
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
        Cable *cab = listCables.at(i);
        if(cab->GetInfoOut().ParentObjectId()==oldObjId) {
            MetaInfo newConnect = cab->GetInfoOut();
            newConnect.SetObjId(newObjId);
            AddCable(newConnect, cab->GetInfoIn());
        }
        if(cab->GetInfoIn().ParentObjectId()==oldObjId) {
            MetaInfo newConnect = cab->GetInfoIn();
            newConnect.SetObjId(newObjId);
            AddCable(cab->GetInfoOut(), newConnect);
        }
        --i;
    }
}

void ContainerProgram::MoveOutputCablesFromObj(int newObjId, int oldObjId)
{
    int i=listCables.size()-1;
    while(i>=0) {
        Cable *cab = listCables.at(i);
        if(cab->GetInfoOut().ParentObjectId() == oldObjId
                && cab->GetInfoOut().Meta(MetaInfos::Media).toInt() != MediaTypes::Parameter) {
            MetaInfo newConnect(cab->GetInfoOut());
            newConnect.SetParentObjectId(newObjId);
            myHost->objFactory->UpdatePinInfo(newConnect);
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
        Cable *cab = listCables.at(i);
        if(cab->GetInfoIn().ParentObjectId()==oldObjId
                && cab->GetInfoIn().Meta(MetaInfos::Media).toInt() != MediaTypes::Parameter) {
            MetaInfo newConnect(cab->GetInfoIn());
            newConnect.SetParentObjectId(newObjId);
            myHost->objFactory->UpdatePinInfo(newConnect);
            if( AddCable(cab->GetInfoOut(), newConnect) ) {
                RemoveCable(cab);
            }
        }
        --i;
    }
}

void ContainerProgram::GetListOfConnectedPinsTo(const MetaInfo &pin, QList<MetaInfo> &list)
{
    int i=listCables.size()-1;
    while(i>=0) {
        Cable *cab = listCables.at(i);
        if(cab->GetInfoIn().ObjId()==pin.ObjId())
            list << cab->GetInfoOut();
        if(cab->GetInfoOut().ObjId()==pin.ObjId())
            list << cab->GetInfoIn();
        --i;
    }
}

bool ContainerProgram::CableExists(const MetaInfo &outputPin, const MetaInfo &inputPin)
{
    foreach(Cable *c, listCables) {
        if(c->GetInfoOut().ObjId()==outputPin.ObjId() && c->GetInfoIn().ObjId()==inputPin.ObjId())
            return true;
    }
    return false;
}

QDataStream & ContainerProgram::toStream (QDataStream& out) const
{
    quint16 nbObj = listObjects.size();
    out << nbObj;
    foreach(QSharedPointer<Object> objPtr, listObjects) {
        out << (qint32)objPtr->ObjId();
    }

    out << (quint16)listCables.size();
    foreach(Cable *cab, listCables) {
        out << *cab;
    }

    out << (quint16)mapObjAttribs.size();
    QMap<int,ObjectContainerAttribs>::ConstIterator i = mapObjAttribs.constBegin();
    while(i!=mapObjAttribs.constEnd()) {
        out << (quint32)i.key();
        out << i.value();
        ++i;
    }

    return out;
}

QDataStream & ContainerProgram::fromStream (QDataStream& in)
{
    quint16 nbobj;
    in >> nbobj;
    for(quint16 i=0; i<nbobj; i++) {
        quint32 id;
        in >> id;
        quint32 newid = MetaInfo::GetIdFromSavedId(id);
//        if(newid==0)
//            return in;
        listObjects << myHost->objFactory->GetObjectFromId(newid);
    }

    quint16 nbCables;
    in >> nbCables;
    for(quint16 i=0; i<nbCables; i++) {
        Cable *cab = new Cable();
        cab->SetTransporter(myHost);
        cab->SetParent(container);
        cab->SetContainer(container);
        in >> *cab;
        cab->UpdatePinsParentIds(myHost);
        listCables << cab;
    }

    quint16 nbPos;
    in >> nbPos;
    for(quint16 i=0; i<nbPos; i++) {
        quint32 objId;
        ObjectContainerAttribs attr;
        in >> objId;
        in >> attr;
        objId=MetaInfo::GetIdFromSavedId(objId);
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


