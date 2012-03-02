/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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

#include "pin.h"
#include "cable.h"
#include "../mainhost.h"

using namespace Connectables;

/*!
  \class Connectables::Pin
  \brief virtual pin
  */

/*!
  Constructor, used by PinsList with the help of Object::CreatePin
  \param parent pointer to the parent Object
  \param type PinType
  \param direction PinDirection
  \param number pin number in the list
  \param bridge true if this pin is a bridge
  */
Pin::Pin(Object *parent,PinType::Enum type, PinDirection::Enum direction, int number, bool bridge) :
    QObject(parent),
    MsgHandler(parent->getHost(), parent->getHost()->objFactory->GetNewObjId()),
    connectInfo(parent->getHost(),parent->GetIndex(),type,direction,number,bridge),
    value(.0f),
    stepSize(.1f),
    parent(parent),
    visible(false),
    closed(false),
    valueChanged(false),
    pinList(0)
{
    connectInfo.container = parent->GetContainerId();
//    setObjectName(QString("pin:%1:%2:%3:%4").arg(connectInfo.objId).arg(connectInfo.type).arg(connectInfo.direction).arg(connectInfo.pinNumber));
}

/*!
  Destructor
  Hide and close
  */
Pin::~Pin()
{
    SetVisible(false);
    Close();
}

/*!
  Send a message to all the connected pins
  \param msgType PinMessage
  \param data data to send
  */
void Pin::SendMsg(const PinMessage::Enum msgType,void *data)
{
    cablesMutex.lock();
    foreach(QSharedPointer<Cable>c, listCables) {
        if(c) {
            c->Render(msgType,data);
        } else {
            listCables.removeAll(c);
        }
    }
    cablesMutex.unlock();
}

/*!
  Update the view with the new parent
  \param newParent
  */
void Pin::SetPinList(PinsList *lst)
{
    pinList=lst;
    SetVisible(true);
}

//void Pin::SetParentModelIndex(PinsList *list, const QModelIndex &newParent)
//{
//    pinList=list;

//    closed=false;

//    //moving from another parent (when does it happen ?)
//    if(parentIndex.isValid() && parentIndex != newParent) {
//        SetVisible(false);
//    }

//    parentIndex=newParent;
//    SetVisible(visible);
//}

/*!
  Set the new container id
  \param id the new container id
  */
void Pin::SetContainerId(quint16 id)
{
    connectInfo.container = id;
}

/*!
  Close the pin before deletion
  \todo can be merged with destructor ?
  */
void Pin::Close()
{
    QMutexLocker l(&objMutex);
    if(parent && parent->getHost() && parent->getHost()->updateViewTimer )
        disconnect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
            this,SLOT(updateView()));
//    parentIndex=QModelIndex();
//    modelIndex=QModelIndex();
    closed=true;
}

/*!
  Set this pin as a bridge (will allow connection in the parent container)
  \param bridge true is it's a bridge
  */
void Pin::SetBridge(bool bridge)
{
    connectInfo.bridge=bridge;
}

void Pin::EnableVuUpdates(bool enab)
{
    if(!parent->getHost()->updateViewTimer)
        return;

    if(visible && enab) {
        connect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
                this,SLOT(updateView()),
                Qt::UniqueConnection);
        return;
    }

    disconnect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
            this,SLOT(updateView()));
}

/*!
  Show or hide the pin
  \param vis true if visible
  */
void Pin::SetVisible(bool vis)
{
    QMutexLocker l(&objMutex);

    visible=vis;

    if(closed)
        return;

    if(!pinList || !pinList->IsVisible())
        return;

    if(!MsgEnabled())
        return;

    if(visible) { // && !modelIndex.isValid()) {
//        QStandardItem *item = new QStandardItem("pin");
//        item->setData(objectName(),Qt::DisplayRole);
//        item->setData(GetValue(),UserRoles::value);
//        item->setData( QVariant::fromValue(ObjectInfo(NodeType::pin)),UserRoles::objInfo);
//        item->setData(QVariant::fromValue(connectInfo),UserRoles::connectionInfo);
//        item->setData(stepSize,UserRoles::stepSize);

//        QStandardItem *parentItem = parent->getHost()->GetModel()->itemFromIndex(parentIndex);
//        parentItem->appendRow(item);
//        modelIndex = item->index();

        MsgObject msg(pinList->GetIndex());
        GetInfos(msg);
        msgCtrl->SendMsg(msg);

        EnableVuUpdates(true);

//        connect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
//                this,SLOT(updateView()),
//                Qt::UniqueConnection);
    }

    if(!visible) {// && modelIndex.isValid()) {
        //        if(connectInfo.type!=PinType::Bridge) {
//                    disconnect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
//                            this,SLOT(updateView()));
        //        }
        EnableVuUpdates(false);
        //remove cables from pin
        QSharedPointer<Object> cnt = parent->getHost()->objFactory->GetObjectFromId(connectInfo.container);
        if(cnt) {
            static_cast<Container*>(cnt.data())->UserRemoveCableFromPin(connectInfo);
        }

        //remove pin
//        if(modelIndex.isValid())
//            parent->getHost()->GetModel()->removeRow(modelIndex.row(), modelIndex.parent());
//        modelIndex=QModelIndex();

        if(pinList) {
            MsgObject msg(pinList->GetIndex());
            msg.prop[MsgObject::Remove]=GetIndex();
            msgCtrl->SendMsg(msg);
        }
    }
}


/*!
  Update view values
  */
void Pin::updateView()
{
    if(closed || !visible) {
        return;
    }

    if(!MsgEnabled())
        return;

    QMutexLocker l(&objMutex);
    MsgObject msg(GetIndex());

    if(!displayedText.isEmpty()) {
        msg.prop[MsgObject::Name]=displayedText;
//        displayedText="";
    }

    float newVu = GetValue();
    if(valueChanged) {
        valueChanged=false;
        msg.prop[MsgObject::Value]=newVu;
    }

    if(msg.prop.count()>0) {
        msgCtrl->SendMsg(msg);
    }
}

void Pin::GetInfos(MsgObject &msg)
{
    msg.prop[MsgObject::Id] = GetIndex();

    msg.prop[MsgObject::Add]=NodeType::pin;
    msg.prop[MsgObject::Value]=value;

    if(displayedText.isEmpty())
        msg.prop[MsgObject::Name]=objectName();
    else
        msg.prop[MsgObject::Name]=displayedText;

    connectInfo.GetInfos(msg);
//    msg.prop["connectionInfo"]=QVariant::fromValue(connectInfo);
}
