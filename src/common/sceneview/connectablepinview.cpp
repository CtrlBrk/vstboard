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

#include "connectablepinview.h"
#include "../connectables/objectfactory.h"

using namespace View;

ConnectablePinView::ConnectablePinView(int listPinId, float angle, MsgController *msgCtrl, int objId, QGraphicsItem * parent, const ConnectionInfo &pinInfo, ViewConfig *config) :
    PinView(listPinId, angle,msgCtrl,objId,parent,pinInfo,config),
    value(0),
    isParameter(false),
    overload(0),
    textItem(0),
    rectVu(0),
    colorGroupId(ColorGroups::ND),
    vuColor(Qt::gray),
    vuFall(.01f)
{
    outline = new QGraphicsRectItem(rect(), this);
    highlight = new QGraphicsRectItem(rect(), this);
    highlight->setVisible(false);

    rectVu = new QGraphicsRectItem(this);
    outline->setBrush(Qt::NoBrush);

    textItem = new QGraphicsSimpleTextItem(this);
    textItem->moveBy(2,1);
    textItem->setZValue(1);

    switch(connectInfo.type) {
    case PinType::Audio :
        colorGroupId=ColorGroups::AudioPin;
        break;
    case PinType::Midi :
        colorGroupId=ColorGroups::MidiPin;
        rectVu->setRect(rect());
        break;
    case PinType::Parameter :
        defaultCursor = Qt::PointingHandCursor;
        setCursor(defaultCursor);
        colorGroupId=ColorGroups::ParameterPin;
        break;
    default :
        colorGroupId=ColorGroups::ND;
        break;
    }

    outline->setBrush( config->GetColor(colorGroupId, Colors::Background) );
    vuColor = config->GetColor(colorGroupId, Colors::VuMeter);
    rectVu->setBrush( vuColor );
    highlight->setBrush( config->GetColor(ColorGroups::Object, Colors::HighlightBackground) );
    textItem->setBrush(  config->GetColor(ColorGroups::Object, Colors::Text) );

    setGeometry(0,0,50,15);
    setMinimumSize(50,15);
    setMaximumSize(80,15);
}

void ConnectablePinView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==colorGroupId && colorId==Colors::Background) {
        outline->setBrush(color);
        return;
    }
    if(groupId==colorGroupId && colorId==Colors::VuMeter) {
        vuColor=color;
        if(connectInfo.type != PinType::Midi) {
            rectVu->setBrush(color);
        }
        return;
    }
    if(groupId==ColorGroups::Object && colorId==Colors::HighlightBackground) {
        highlight->setBrush(color);
        return;
    }

    if(groupId==ColorGroups::Object && colorId==Colors::Text) {
        textItem->setBrush(color);
        return;
    }
}

void ConnectablePinView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    PinView::resizeEvent(event);

    if(outline) {
        QRectF r = outline->rect();
        r.setSize(event->newSize());
        outline->setRect(r);
    }
    if(highlight){
        QRectF r = highlight->rect();
        r.setSize(event->newSize());
        highlight->setRect(r);
    }
    if(rectVu) {
        QRectF r = rectVu->rect();
        if(connectInfo.type == PinType::Midi) {
            r.setSize(event->newSize());
            rectVu->setRect(r);
        } else if(value>.0f) {
            r.setWidth(value*event->newSize().width());
            r.setHeight(event->newSize().height());
            rectVu->setRect(r);
        }
    }

    UpdateCablesPosition();
}

void ConnectablePinView::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    ResetOveload();
    PinView::mousePressEvent(event);
}

void ConnectablePinView::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Name)) {
        QString newName = msg.prop[MsgObject::Name].toString();
        if(newName!=textItem->text()) {
            textItem->setText(newName);

            if(textItem->boundingRect().width()>size().width())
                setMinimumWidth( textItem->boundingRect().width()+10 );
        }
    }

    if(msg.prop.contains(MsgObject::Value)) {
        if(connectInfo.type == PinType::Parameter) {
            value = msg.prop[MsgObject::Value].toFloat();
            float newVu = size().width() * value;
            rectVu->setRect(0,0, newVu, size().height());
        } else {
            float newVal = msg.prop[MsgObject::Value].toFloat();
            if(newVal>value) {
                vuFall=.002f;
                value=newVal;
            }
        }
    }

//    PinView::ReceiveMsg(msg);
}

void ConnectablePinView::ResetOveload()
{
    overload=0;
    rectVu->setBrush(vuColor);
}

void ConnectablePinView::updateVu()
{
    if(value<.0f)
        return;

    if(connectInfo.type==PinType::Audio) {
        value-=vuFall;
        vuFall*=2.0f;
        if(value>1.0f) {
            if(overload==0) {
                rectVu->setBrush(Qt::red);
                overload=500;
            }
            value=1.0f;
        }

        if(overload>0) {
            overload--;
            if(overload==0) {
                ResetOveload();
            }
        }

        float newVu=.0f;
        if(value<.0f) {
            value=-1.0f;
        } else {
            newVu = size().width() * value;
        }
        if(newVu<0.0f) {
            LOG("updateVu <0"<<newVu);
            newVu=0.0f;
        }

        rectVu->setRect(0,0, newVu, size().height());
    }

    if(connectInfo.type== PinType::Midi) {
        value-=.1f;
        if(value<.0f) {
            value=-1.0f;
            rectVu->setBrush(Qt::NoBrush);
            return;
        }
        QColor c = vuColor;

        if(value<0.7f)
            c.setAlphaF( value/0.7f );

        rectVu->setBrush(c);
    }
}

