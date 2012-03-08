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

#include "maincontainerview.h"
#include "../connectables/objectfactory.h"
#include "objectview.h"
#include "vstpluginview.h"

using namespace View;

MainContainerView::MainContainerView(ViewConfig *config, MsgController *msgCtrl, int objId) :
        ObjectView(config,msgCtrl,objId)
{
    content = new ContainerContent(config,msgCtrl,-1,this);
    content->setAcceptDrops(true);
    connect(content, SIGNAL(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)),
            this,SLOT(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)));

    bridgeIn = new BridgeView(config, msgCtrl, -1, this);
    bridgeOut = new BridgeView(config, msgCtrl, -1, this);
    bridgeSend = new BridgeView(config, msgCtrl, -1, this);
    bridgeReturn = new BridgeView(config, msgCtrl, -1, this);

    setGeometry(0,0,0,0);
}

MainContainerView::~MainContainerView()
{
    foreach(CableView *c, listCables) {
        c->scene()->removeItem(c);
        c->deleteLater();
    }
}

void MainContainerView::showEvent(QShowEvent *event)
{
//    LOG("container show event"<<GetIndex())
}

QPointF MainContainerView::GetDropPos()
{
    return mapFromScene( content->GetDropPos() );
}

void MainContainerView::SetDropPos(const QPointF &pt)
{
    content->SetDropPos( pt );
}

void MainContainerView::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Add)) {
        switch(msg.prop[MsgObject::Add].toInt()) {
        case NodeType::bridge :
            AddBridge(msg);
            break;

        case NodeType::object :
            AddObject(msg);
            break;

        case NodeType::cable :
            AddCable(msg);
            break;

        case NodeType::tempObj :
            AddTempObj(msg);
            break;
        }
    }
//    content->ReceiveMsg(msg);
}


void MainContainerView::AddBridge(const MsgObject &msg)
{
    ObjectView *objView = 0;

    switch(msg.prop[MsgObject::Type].toInt()) {
        case ObjType::BridgeIn :
            objView = bridgeIn;
            break;
        case ObjType::BridgeOut :
            objView = bridgeOut;
            break;
        case ObjType::BridgeSend :
            objView = bridgeSend;
            break;
        case ObjType::BridgeReturn :
            objView = bridgeReturn;
            break;
        default:
            LOG("unknown listpin");
            return;
    }

    objView->SetIndex( msg.prop[MsgObject::Id].toInt() );
    objView->ReceiveMsg(msg);
}

void MainContainerView::AddObject(const MsgObject &msg)
{
    ConnectableObjectView *objView = 0;
    QPointF pos = GetDropPos();
    int objId = msg.prop[MsgObject::Id].toInt();

    //a temp obj exists
    if(msgCtrl->listObj.contains(objId)) {
        ConnectableObjectView *obj = static_cast<ConnectableObjectView*>( msgCtrl->listObj.value(objId) );
        pos=obj->pos();
        delete obj;
    }

    if(msg.prop[MsgObject::Type].toInt() == ObjType::VstPlugin) {
        objView = new VstPluginView(config, msgCtrl, objId, this);
    } else {
        objView = new ConnectableObjectView(config, msgCtrl, objId, this);
    }
    objView->Init(msg);

    if(msg.prop.contains(MsgObject::State)) {
        ObjectContainerAttribs attr = msg.prop[MsgObject::State].value<ObjectContainerAttribs>();
        objView->setPos(attr.position);
    } else {
        objView->setPos(pos);
        objView->ReportPosChange();
    }

    //when adding item, the scene set focus to the last item
    if(scene()->focusItem())
        scene()->focusItem()->clearFocus();
}


void MainContainerView::AddCable(const MsgObject &msg)
{
    PinView* pinOut = static_cast<PinView*>(msgCtrl->listObj.value(msg.prop[MsgObject::PinOut].toInt(),0));
    PinView* pinIn = static_cast<PinView*>(msgCtrl->listObj.value(msg.prop[MsgObject::PinIn].toInt(),0));

    if(!pinOut || !pinIn) {
        LOG("addcable : pin not found"<<msg.prop);
        return;
    }
    CableView *cable = new CableView(msgCtrl, msg.prop[MsgObject::Id].toInt(), pinOut, pinIn, this, config);
    cable->ReceiveMsg(msg);
    pinOut->AddCable(cable);
    pinIn->AddCable(cable);
    cable->ReceiveMsg(msg);
}

void MainContainerView::AddTempObj(const MsgObject &msg)
{

    ObjectView *objView = new ConnectableObjectView(config, msgCtrl, msg.prop[MsgObject::Id].toInt(), this);
    objView->ReceiveMsg(msg);
    objView->setPos(GetDropPos());

    //when adding item, the scene set focus to the last item
    if(scene()->focusItem())
        scene()->focusItem()->clearFocus();
}

//void MainContainerView::SetModelIndex(QPersistentModelIndex index)
//{
//    setObjectName(QString("MainContainerView%1").arg(index.data(UserRoles::value).toInt()));
//    content->SetModelIndex(index);
//}

void MainContainerView::SetParking(QWidget *parking)
{
    content->SetParking( parking );
    emit ParkingChanged(parking);
}

QWidget *MainContainerView::GetParking()
{
    return content->GetParking();
}

void MainContainerView::OnViewChanged(QRectF rect)
{
    bridgeIn->setPos(rect.topLeft());
    bridgeOut->setPos(rect.right() - bridgeOut->geometry().width(), rect.top());
    bridgeSend->setPos(rect.left(), rect.bottom() - bridgeSend->geometry().height() );
    bridgeReturn->setPos(rect.right() - bridgeReturn->geometry().width(), rect.bottom() - bridgeReturn->geometry().height());
    content->setGeometry(rect);
}

void MainContainerView::ObjectDropped(QGraphicsSceneDragDropEvent *event, MsgObject msg)
{
    SetDropPos( event->scenePos() );
    msg.objIndex=GetIndex();
    msg.prop[MsgObject::Type]=InsertionType::NoInsertion;
    msgCtrl->SendMsg(msg);
}
