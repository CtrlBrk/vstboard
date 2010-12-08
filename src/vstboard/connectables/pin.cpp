/******************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
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
******************************************************************************/

#include "pin.h"
#include "../mainhost.h"

using namespace Connectables;

Pin::Pin(Object *parent,PinType::Enum type, PinDirection::Enum direction, int number, bool bridge) :
    QObject(parent),
    connectInfo(parent->getHost(),parent->GetIndex(),type,direction,number,bridge),
    value(.0f),
    falloff(.05f),
    stepSize(.1f),
    parent(parent),
    visible(false),
    closed(false),
    valueChanged(false)
{
//    setObjectName(QString("pin:%1:%2:%3:%4").arg(connectInfo.objId).arg(connectInfo.type).arg(connectInfo.direction).arg(connectInfo.pinNumber));
}

Pin::~Pin()
{
    SetVisible(false);
    Close();
}

void Pin::setObjectName(const QString &name)
{
    if(modelIndex.isValid())
        parent->getHost()->GetModel()->setData(modelIndex,name,Qt::DisplayRole);

    QObject::setObjectName(name);
}

bool Pin::event(QEvent *event)
{
    if (event->type() == Event::PinMessage) {
        PinMessageEvent *e = static_cast<PinMessageEvent *>(event);
        ReceiveMsg(e->msgType,e->data);
        return true;
    }
    return QObject::event(event);
}

void Pin::SendMsg(int msgType,void *data)
{
    parent->getHost()->SendMsg(connectInfo,(PinMessage::Enum)msgType,data);
}

//QString Pin::GetDisplayedText()
//{
//    QString retStr;
//    txtMutex.lock();
//    retStr = displayedText;
//    txtMutex.unlock();
//    return retStr;
//}

//void Pin::SetDisplayedText(const QString &txt)
//{
//    txtMutex.lock();
//    displayedText = txt;
//    txtMutex.unlock();
//}

void Pin::SetParentModelIndex(const QModelIndex &newParent)
{
    closed=false;

    if(parentIndex.isValid()) {
        if(visible) {
        //should be visible
            if(parentIndex == newParent) {
                //same parent : update existing model node (happens when a errorenous device becomes available)
                UpdateModelNode();
                return;
            } else {
                //moving from another parent (when does it happen ?)
                SetVisible(false);
                parentIndex=newParent;
                SetVisible(true);
                return;
            }
        }
    }

    parentIndex=newParent;
    if(visible) {
        visible=false;
        SetVisible(true);
    }
}

void Pin::SetContainerId(quint16 id)
{
    connectInfo.container = id;
}

void Pin::Close()
{
    QMutexLocker l(&objMutex);
    disconnect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
            this,SLOT(updateView()));
    parentIndex=QModelIndex();
    modelIndex=QModelIndex();
    closed=true;
}

void Pin::SetVisible(bool vis)
{
    if(visible==vis)
        return;

    QMutexLocker l(&objMutex);

    visible=vis;

    if(closed)
        return;

    if(!parentIndex.isValid())
        return;

    if(visible) {
        QStandardItem *item = new QStandardItem("pin");
        item->setData(objectName(),Qt::DisplayRole);
        item->setData(falloff,UserRoles::falloff);
        item->setData(GetValue(),UserRoles::value);
        item->setData( QVariant::fromValue(ObjectInfo(NodeType::pin)),UserRoles::objInfo);
        item->setData(QVariant::fromValue(connectInfo),UserRoles::connectionInfo);
        item->setData(stepSize,UserRoles::stepSize);

        QStandardItem *parentItem = parent->getHost()->GetModel()->itemFromIndex(parentIndex);
        parentItem->appendRow(item);
        modelIndex = item->index();
        if(connectInfo.type!=PinType::Bridge) {
            connect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
                    this,SLOT(updateView()),
                    Qt::UniqueConnection);
        }
    } else {
        if(modelIndex.isValid()) {
            //remove cables from pin
            QSharedPointer<Object> cnt = parent->getHost()->objFactory->GetObjectFromId(connectInfo.container);
            if(!cnt.isNull()) {
                static_cast<Container*>(cnt.data())->RemoveCableFromPin(connectInfo);
            }

            //remove pin
            if(connectInfo.type!=PinType::Bridge) {
                disconnect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
                        this,SLOT(updateView()));
            }

            if(modelIndex.isValid())
                parent->getHost()->GetModel()->removeRow(modelIndex.row(), modelIndex.parent());
            modelIndex=QModelIndex();
        }
    }
}

void Pin::UpdateModelNode()
{
    QStandardItem *item = parent->getHost()->GetModel()->itemFromIndex(parentIndex)->child(connectInfo.pinNumber,0);
    if(!item) {
        debug("Pin::UpdateModelNode can't find item")
        return;
    }
    item->setData(objectName(),Qt::DisplayRole);
    item->setData(falloff,UserRoles::falloff);
    item->setData(GetValue(),UserRoles::value);
    item->setData(QVariant::fromValue(ObjectInfo(NodeType::pin)),UserRoles::objInfo);
    item->setData(QVariant::fromValue(connectInfo),UserRoles::connectionInfo);
    item->setData(stepSize,UserRoles::stepSize);
    //MainHost::GetModel()->itemFromIndex(parentIndex)->appendRow(item);
    modelIndex = item->index();
    if(connectInfo.type!=PinType::Bridge) {
        connect(parent->getHost()->updateViewTimer,SIGNAL(timeout()),
                this,SLOT(updateView()),
                Qt::UniqueConnection);
    }
}

void Pin::updateView()
{
    QMutexLocker l(&objMutex);

    if(closed || !visible | !modelIndex.isValid()) {
        return;
    }

    float newVu = GetValue();
    if(!valueChanged)
        return;

    valueChanged=false;

    parent->getHost()->GetModel()->setData(modelIndex, newVu, UserRoles::value);
    if(!displayedText.isEmpty()) parent->getHost()->GetModel()->setData(modelIndex, displayedText, Qt::DisplayRole);

}
