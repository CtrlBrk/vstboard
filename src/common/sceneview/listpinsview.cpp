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

#include "listpinsview.h"
#include "cableview.h"

#include "connectablepinview.h"
#include "minmaxpinview.h"
#include "bridgepinview.h"

using namespace View;

ListPinsView::ListPinsView(ViewConfig *config, MsgController *msgCtrl, int objId, QGraphicsItem * parent) :
    QGraphicsWidget(parent),
    MsgHandler(msgCtrl, objId),
    config(config)
{
    setObjectName("listPinView");
    layout = new QGraphicsLinearLayout(this);
    layout->setOrientation(Qt::Vertical);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

}

void ListPinsView::UpdateCablesPosition()
{
    for(int i=0; i<layout->count(); i++) {
        static_cast<PinView*>(layout->itemAt(i))->UpdateCablesPosition();
    }
}

int ListPinsView::GetPinPosition(int newPinNumber)
{
    for(int i=0; i<layout->count(); i++) {
        if( static_cast<PinView*>(layout->itemAt(i))->GetConnectionInfo().pinNumber > newPinNumber) {
            return i;
        }
    }
    return layout->count();
}

//#ifndef QT_NO_DEBUG
//void ListPinsView::SetIndex(int id)
//{
//    MsgHandler::SetIndex(id);
//    new QGraphicsSimpleTextItem(QString("pinLst %1").arg(id),this);
//}
//#endif

void ListPinsView::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Add)) {
        switch(msg.prop[MsgObject::Add].toInt()) {
        case NodeType::pin :
            AddPin(msg);
            break;
        }
    }
}

void ListPinsView::AddPin(const MsgObject &msg)
{
    ConnectionInfo pinInfo(msg);// = msg.prop.value("connectionInfo").value<ConnectionInfo>();
    PinView *pinView;
    float angle=.0f;

    if(msg.prop[MsgObject::ParentNodeType].toInt() == NodeType::bridge) {
        ObjType::Enum t = (ObjType::Enum)msg.prop[MsgObject::ParentObjType].toInt();
        if(t==ObjType::BridgeIn || t==ObjType::BridgeOut)
            angle=1.570796f; //pi/2
        if(t==ObjType::BridgeSend || t==ObjType::BridgeReturn)
            angle=-1.570796f; //-pi/2

        pinView = static_cast<PinView*>( new BridgePinView(GetIndex(), angle, msgCtrl, msg.prop[MsgObject::Id].toInt(), this, pinInfo,config) );

    } else {
        if(pinInfo.direction==PinDirection::Input)
            angle=3.141592f;
        if(pinInfo.direction==PinDirection::Output)
            angle=.0f;

        if(pinInfo.type==PinType::Parameter) {
            pinView = new MinMaxPinView(GetIndex(), angle, msgCtrl, msg.prop[MsgObject::Id].toInt(), this, pinInfo, config);
        } else {
            pinView = new ConnectablePinView(GetIndex(), angle, msgCtrl, msg.prop[MsgObject::Id].toInt(), this, pinInfo, config);
        }
    }

    connect(config->timerFalloff,SIGNAL(timeout()),
        pinView,SLOT(updateVu()));

//            mapConnectionInfo.insert(pinInfo,index);

    int pinPlace = GetPinPosition(pinInfo.pinNumber);
    layout->insertItem(pinPlace, pinView);

    layout->setAlignment(pinView,Qt::AlignTop);
//                connect(pinView, SIGNAL(ConnectPins(ConnectionInfo,ConnectionInfo)),
//                        this, SLOT(ConnectPins(ConnectionInfo,ConnectionInfo)));
//                connect(pinView,SIGNAL(RemoveCablesFromPin(ConnectionInfo)),
//                        this,SLOT(RemoveCablesFromPin(ConnectionInfo)));
//                connect(pinView,SIGNAL(RemovePin(ConnectionInfo)),
//                        this,SLOT(RemovePin(ConnectionInfo)));

    pinView->ReceiveMsg(msg);
}
