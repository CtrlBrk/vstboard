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


#include "pinslist.h"
#include "pin.h"
#include "audiobuffer.h"
#include "audiopin.h"
#include "midipinin.h"
#include "midipinout.h"
#include "bridgepinin.h"
#include "bridgepinout.h"
#include "mainhost.h"

#include "commands/comremovepin.h"

using namespace Connectables;

PinsList::PinsList(MainHost *myHost, Object *parent, MsgController *msgCtrl, int objId) :
    QObject(parent),
    MsgHandler(msgCtrl,objId),
    parent(parent),
    myHost(myHost),
    visible(true)
{
    connect(this,SIGNAL(PinAdded(int)),
            this,SLOT(AddPin(int)));
    connect(this,SIGNAL(PinRemoved(int)),
            this,SLOT(RemovePin(int)));
//    connect(this, SIGNAL(NbPinChanged(int)),
//            this,SLOT(SetNbPins(int)));
}

PinsList::~PinsList()
{
    foreach(Pin* pin, listPins) {
        if(pin) {
            pin->SetPinList(0);
        }
    }
}

void PinsList::SetContainerId(quint16 id)
{
    connInfo.container=id;
    foreach(Pin* pin, listPins) {
        if(pin) {
            pin->SetContainerId(id);
        }
    }
}

void PinsList::Hide()
{
    foreach(Pin* pin, listPins) {
        if(pin) {
            pin->Close();
        }
    }
}

void PinsList::ChangeNumberOfPins(int newNb)
{
    emit SetNbPins(newNb);
}

void PinsList::SetInfo(Object *parent,const ConnectionInfo &connInfo, const ObjectInfo &objInfo)
{
    this->parent=parent;
    this->connInfo=connInfo;
    this->objInfo=objInfo;
}

//void PinsList::SetVisible(bool v)
//{
//    visible=v;
//    foreach(Pin* pin, listPins) {
//        pin->SetVisible(visible);
//    }
//}

void PinsList::EnableVuUpdates(bool enab)
{
    foreach(Pin* pin, listPins) {
        if(pin) {
            pin->EnableVuUpdates(enab);
        }
    }
}

void PinsList::SetBridge(bool bridge)
{
    connInfo.bridge=bridge;
    visible=!bridge;
    foreach(Pin* pin, listPins) {
        if(pin) {
            pin->SetBridge(bridge);
            pin->SetVisible(visible);
        }
    }
}

Pin * PinsList::GetPin(int pinNumber, bool autoCreate)
{
    //resize the list if needed
    if(!listPins.contains(pinNumber)) {
        if(autoCreate) {
            AddPin(pinNumber);
        } else {
            LOG("pin not in list"<<pinNumber);
            return 0;
        }
    }

    return listPins.value(pinNumber);
}

AudioBuffer *PinsList::GetBuffer(int pinNumber)
{
    if(connInfo.type != PinType::Audio)
        return 0;

    if(!listPins.contains(pinNumber)) {
        LOG("pin not found"<<pinNumber);
        return 0;
    }

    return static_cast<AudioPin*>(listPins.value(pinNumber))->GetBuffer();
}

void PinsList::ConnectAllTo(Container* container, PinsList *other, bool hidden)
{
//    QSharedPointer<Object>cntPtr = myHost->objFactory->GetObj(modelList.parent().parent());

    QMap<quint16,Pin*>::Iterator i = listPins.begin();
    while(i!=listPins.end()) {
        Pin *otherPin = other->listPins.value(i.key(),0);
        if(otherPin)
            container->AddCable(i.value()->GetConnectionInfo(),otherPin->GetConnectionInfo(),hidden);
        ++i;
    }
}

void PinsList::AsyncAddPin(int nb)
{
    if(listPins.contains(nb))
        return;
    emit PinAdded(nb);
}

void PinsList::AsyncRemovePin(int nb)
{
    if(!listPins.contains(nb))
        return;
    emit PinRemoved(nb);
}

void PinsList::SetNbPins(int nb, QList<quint16> *listAdded,QList<quint16> *listRemoved)
{
    QMap<quint16,Pin*>::iterator i = listPins.begin();
    while(i!=listPins.end()) {
        if(i.key()>=nb && i.key()<FIXED_PIN_STARTINDEX) {
            if(listRemoved) {
                *listRemoved << i.key();
                ++i;
            } else {
                RemovePin(i.key());
                i=listPins.erase(i);
            }
        } else {
            ++i;
        }
    }

    int cpt=0;
    while(cpt<nb) {
        if(!listPins.contains(cpt)) {
            if(listAdded) {
                *listAdded<<cpt;
            } else {
                AddPin(cpt);
            }
        }
        cpt++;
    }
}

Pin * PinsList::AddPin(int nb)
{
    if(listPins.contains(nb))
        return listPins.value(nb);

    connInfo.pinNumber=nb;
    Pin *newPin = parent->CreatePin(connInfo);

    if(!newPin) {
        LOG("pin not created"<<nb);
        return 0;
    }
    newPin->SetMsgEnabled(MsgEnabled());
    listPins.insert(nb, newPin);

    newPin->SetPinList(this);
//    if(modelList.isValid())
//        newPin->SetParentModelIndex(this,modelList);

    parent->OnProgramDirty();
    return newPin;
}

void PinsList::RemovePin(int nb)
{
    if(!listPins.contains(nb))
        return;

    parent->OnProgramDirty();
    delete listPins.take(nb);

}

QDataStream & PinsList::toStream(QDataStream & out) const
{
    out << connInfo;
    out << objInfo;

    out << (quint16)listPins.count();

    QMap<quint16,Pin*>::ConstIterator i = listPins.constBegin();
    while(i!=listPins.constEnd()) {
        Pin *pin=i.value();
        out << i.key();
        out << pin->GetValue();
        ++i;
    }

    return out;
}

QDataStream & PinsList::fromStream(QDataStream & in)
{
    in >> connInfo;
    in >> objInfo;

    quint16 nbPins;
    in >> nbPins;

    for(quint16 i=0; i<nbPins; i++) {
        quint16 id;
        in >> id;
        QVariant value;
        in >> value;
        connInfo.pinNumber=id;
        Pin *newPin = parent->CreatePin(connInfo);
        newPin->SetMsgEnabled(MsgEnabled());
        if(!newPin) {
            LOG("pin not created"<<id);
            return in;
        }
        listPins.insert(id,newPin);

    }

    return in;
}

QDataStream & operator<< (QDataStream & out, const Connectables::PinsList& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, Connectables::PinsList& value)
{
    return value.fromStream(in);
}

void PinsList::GetInfos(MsgObject &msg)
{
    msg.prop[MsgObject::Id]=GetIndex();
    msg.prop[MsgObject::Add]=NodeType::listPin;
    msg.prop[MsgObject::Type]=objInfo.objType;
    msg.prop[MsgObject::Name]=objectName();

    QMap<quint16,Pin*>::const_iterator i = listPins.constBegin();
    while(i!=listPins.constEnd()) {
        Pin *p = i.value();
        if(p) {
            if(!p->GetVisible()) {
                ++i;
                continue;
            }
//            MsgObject msgPin(GetIndex());
            _MSGOBJ(msgPin,GetIndex());
            msgPin.prop[MsgObject::ParentNodeType]=parent->info().nodeType;
            msgPin.prop[MsgObject::ParentObjType]=parent->info().objType;
            p->GetInfos(msgPin);
            msg.children << msgPin;
        }
        ++i;
    }
}

void PinsList::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::RemovePin)) {
        ConnectionInfo cPin(msg);
        myHost->undoStack.push( new ComRemovePin(myHost, cPin) );
        return;
    }
}

void PinsList::SetMsgEnabled(bool enab)
{
    MsgHandler::SetMsgEnabled(enab);
    foreach(Pin *pin, listPins) {
        if(pin) {
            pin->SetMsgEnabled(enab);
        }
    }
}
