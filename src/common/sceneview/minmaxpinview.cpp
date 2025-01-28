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
//#include "precomp.h"
#include "minmaxpinview.h"
#include "connectableobjectview.h"

using namespace View;

MinMaxPinView::MinMaxPinView(int listPinId, float angle, MsgController *msgCtrl, int objId, QGraphicsItem * parent, const ConnectionInfo &pinInfo, ViewConfig *config) :
    ConnectablePinView(listPinId,angle,msgCtrl,objId,parent,pinInfo,config),
    inMin(0),
    inMax(0),
    outMin(0),
    outMax(0),
    scaledView(0),
    changingValue(false),
    startDragValue(.0f),
    parentObjType(ObjType::ND),
    nbValues(0)
{
    CreateCursors();
}

void MinMaxPinView::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Add)) {
        switch(msg.prop[MsgObject::Add].toInt()) {
        case NodeType::cursor :
            AddCursor(msg);
            return;
        }
    }

    ConnectablePinView::ReceiveMsg(msg);

    if(msg.prop.contains(MsgObject::Row)) {
        nbValues = msg.prop[MsgObject::Row].toInt();
    }

    if(msg.prop.contains(MsgObject::Value)) {
        UpdateScaleView();

        ConnectableObjectView *parentObj = static_cast<ConnectableObjectView*>(parentWidget()->parentWidget());
        if(parentObj) {
            if(connectInfo.pinNumber == FixedPinNumber::editorVisible) {
                parentObj->SetEditorPin(this, value);
            }
            if(connectInfo.pinNumber == FixedPinNumber::learningMode) {
                parentObj->SetLearnPin(this, value);
            }
            if(connectInfo.pinNumber == FixedPinNumber::bypass) {
                parentObj->SetBypassPin(this, value);
            }

            parentObjType = parentObj->objType;
        }
    }
}

void MinMaxPinView::AddCursor(const MsgObject &msg)
{
    switch(msg.prop[MsgObject::Type].toInt()) {
        case ObjType::limitInMin :
            inMin->SetIndex(msg.prop[MsgObject::Id].toInt());
            inMin->SetValue(msg.prop[MsgObject::Value].toFloat());
            break;
        case ObjType::limitInMax :
            inMax->SetIndex(msg.prop[MsgObject::Id].toInt());
            inMax->SetValue(msg.prop[MsgObject::Value].toFloat());
            break;
        case ObjType::limitOutMin :
            outMin->SetIndex(msg.prop[MsgObject::Id].toInt());
            outMin->SetValue(msg.prop[MsgObject::Value].toFloat());
            break;
        case ObjType::limitOutMax :
            outMax->SetIndex(msg.prop[MsgObject::Id].toInt());
            outMax->SetValue(msg.prop[MsgObject::Value].toFloat());
            break;
    }
}

void MinMaxPinView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    ConnectablePinView::resizeEvent(event);

    inMin->setPos( inMin->GetValue()*event->newSize().width(), 0 );
    inMax->setPos( inMax->GetValue()*event->newSize().width(), 0 );
    outMin->setPos( outMin->GetValue()*event->newSize().width(), 0 );
    outMax->setPos( outMax->GetValue()*event->newSize().width(), 0 );
    UpdateScaleView();
}

void MinMaxPinView::CreateCursors()
{
//    cursorCreated=true;

    scaledView = new QGraphicsPolygonItem(this);
    scaledView->setBrush(QColor(0,0,0,30));

    inMin=new CursorView(msgCtrl,-1,false,false,this,config);
    inMin->setPos(rect().topLeft());
    inMin->SetValue(.0f);
    connect(inMin,SIGNAL(ValueChanged()),
            this,SLOT(CursorValueChanged()));

    inMax=new CursorView(msgCtrl,-1,true,false,this,config);
    inMax->setPos(rect().topRight());
    inMax->SetValue(1.0f);
    connect(inMax,SIGNAL(ValueChanged()),
            this,SLOT(CursorValueChanged()));

    outMin=new CursorView(msgCtrl,-1,false,true,this,config);
    outMin->setPos(rect().bottomLeft());
    outMin->SetValue(.0f);
    connect(outMin,SIGNAL(ValueChanged()),
            this,SLOT(CursorValueChanged()));

    outMax=new CursorView(msgCtrl,-1,true,true,this,config);
    outMax->setPos(rect().bottomRight());
    outMax->SetValue(1.0f);
    connect(outMax,SIGNAL(ValueChanged()),
            this,SLOT(CursorValueChanged()));
}

void MinMaxPinView::SetCursor(ObjType::Enum type, QPersistentModelIndex index)
{
//    if(!cursorCreated)
//        CreateCursors();

    switch(type) {
        case ObjType::limitInMin:
            inMin->SetModelIndex(index);
            break;
        case ObjType::limitInMax:
            inMax->SetModelIndex(index);
            break;
        case ObjType::limitOutMin:
            outMin->SetModelIndex(index);
            break;
        case ObjType::limitOutMax:
            outMax->SetModelIndex(index);
            break;
        default:
            LOG("unknown type"<<type);
            break;
    }
}

void MinMaxPinView::CursorValueChanged()
{
    UpdateScaleView();
}

void MinMaxPinView::UpdateLimitModelIndex(const QModelIndex &index)
{
    inMin->SetValue( index.model()->index(0, 0, index).data(UserRoles::value).toFloat() );
    inMax->SetValue( index.model()->index(1,0, index).data(UserRoles::value).toFloat() );
    outMin->SetValue( index.model()->index(2,0, index).data(UserRoles::value).toFloat() );
    outMax->SetValue( index.model()->index(3,0, index).data(UserRoles::value).toFloat() );
    UpdateScaleView();
}

void MinMaxPinView::UpdateScaleView()
{
    float limitVal=value;
    if(limitVal<inMin->GetValue())
        limitVal=inMin->GetValue();
    if(limitVal>inMax->GetValue())
        limitVal=inMax->GetValue();

    float outVal=limitVal;
    outVal-=inMin->GetValue();

    if(inMax->GetValue()!=inMin->GetValue())
        outVal/=(inMax->GetValue()-inMin->GetValue());
    outVal*=(outMax->GetValue()-outMin->GetValue());
    outVal+=outMin->GetValue();

    limitVal*=rect().width();
    outVal*=rect().width();

    QPolygonF pol;
    pol << QPointF(limitVal,0) << QPointF(inMin->GetValue(),0)  << QPointF(outMin->GetValue(),rect().height()) << QPointF(outVal,rect().height());
    scaledView->setPolygon(pol);
}

void MinMaxPinView::ValueChanged(float newVal)
{
    if(value==newVal)
        return;
    if(newVal>1.0f) newVal=1.0f;
    if(newVal<0.0f) newVal=0.0f;

    MSGOBJ();
    msg.prop[MsgObject::Type]=parentObjType;
    msg.prop[MsgObject::Id]=connectInfo.pinNumber;
    msg.prop[MsgObject::Value]=newVal;
    msgCtrl->SendMsg(msg);
}

void MinMaxPinView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    contextMenuEnabled = true;

    const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::changeValue);
    if(b.input == KeyBind::mouse && b.buttons == event->buttons() && b.modifier == event->modifiers()) {
        changingValue=true;
        startDragPos=event->pos();
        startDragValue=value;//pinIndex.data(UserRoles::value).toFloat();
//        grabMouse();
        return;
    }

//    if(config->EditMode()!=EditMode::Value) {
        ConnectablePinView::mousePressEvent(event);
        return;
//    }

//    changingValue=true;
//    startDragPos=event->screenPos();
//    startDragValue=pinIndex.data(UserRoles::value).toFloat();
//    grabMouse();
}

void MinMaxPinView::mouseMoveEvent ( QGraphicsSceneMouseEvent  * event )
{
    if(changingValue) {
        contextMenuEnabled = false;

        float _mouseSensitivity = 1.0f/size().width();
        if(event->modifiers() & Qt::ControlModifier)
            _mouseSensitivity /= 10.0f;
        if(event->modifiers() & Qt::ShiftModifier)
            _mouseSensitivity /= 10.0f;
        if(event->modifiers() & Qt::AltModifier)
            _mouseSensitivity /= 10.0f;

        // if(nbValues!=0 && nbValues<20) {
        //     _mouseSensitivity *= 5.0f;
        // }

        qreal increm = event->pos().x() - startDragPos.x();
        startDragValue += _mouseSensitivity*increm;
        startDragValue = std::max(.0f,startDragValue);
        startDragValue = std::min(1.0f,startDragValue);

        startDragPos=event->pos();
        ValueChanged( startDragValue );
    } else {
        ConnectablePinView::mouseMoveEvent(event);
    }
}

void MinMaxPinView::mouseReleaseEvent ( QGraphicsSceneMouseEvent  * event )
{
    if(changingValue) {
        event->accept();
        ungrabMouse();
        changingValue=false;
    }
    ConnectablePinView::mouseReleaseEvent(event);
}

void MinMaxPinView::wheelEvent ( QGraphicsSceneWheelEvent * event )
{
    const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::changeValue);
    Qt::KeyboardModifiers mod = event->modifiers();
    if(b.input == KeyBind::mouseWheel && (b.modifier == Qt::NoModifier || b.modifier == mod )) {
        event->accept();

        int increm=1;
        if(event->delta()<0)
            increm=-1;

        if(mod & Qt::ControlModifier && mod != b.modifier)
            increm *= 10.0f;
        if(mod & Qt::ShiftModifier && mod != b.modifier)
            increm *= 10.0f;
        if(mod & Qt::AltModifier && mod != b.modifier)
            increm *= 10.0f;

        MSGOBJ();
        msg.prop[MsgObject::Increment]=increm;
        msgCtrl->SendMsg(msg);
//        ValueChanged( pinIndex.data(UserRoles::value).toFloat()
//                      + pinIndex.data(UserRoles::stepSize).toFloat()*increm);
    }
}

void MinMaxPinView::keyPressEvent ( QKeyEvent * event )
{
    int k = event->key();

    if(connectInfo.type == PinType::Parameter) {
        if(event->modifiers() & Qt::ControlModifier) {
            if(k==Qt::Key_Left) { ValueChanged(value-0.01); return; }
            if(k==Qt::Key_Right) { ValueChanged(value+0.01); return; }
        } else {
            if(k==Qt::Key_Left) { ValueChanged(value-0.1); return; }
            if(k==Qt::Key_Right) { ValueChanged(value+0.1); return; }
        }

        float val = ViewConfig::KeyboardNumber(k);
        if(val>=0) {
            ValueChanged(val);
            return;
        }
    }

    QGraphicsWidget::keyPressEvent(event);
}
