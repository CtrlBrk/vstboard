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

#include "cableview.h"

using namespace View;

CableView::CableView(MsgController *msgCtrl, int objId, PinView *pinOut, PinView *pinIn, QGraphicsItem *parent, ViewConfig *config)
#ifndef SIMPLE_CABLES
    : QGraphicsPathItem(parent),
#else
    : QGraphicsLineItem(parent),
#endif

    QObject(),
    MsgHandler(msgCtrl,objId),
    pinOut(pinOut),
    pinIn(pinIn)
{
    setPen( config->GetColor(ColorGroups::Panel,Colors::Lines) );
    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );
}

CableView::CableView(MsgController *msgCtrl, int objId, PinView *pinOut, const QPointF &PtIn, QGraphicsItem *parent, ViewConfig *config)
#ifndef SIMPLE_CABLES
    : QGraphicsPathItem(parent),
#else
    : QGraphicsLineItem(parent),
#endif
    QObject(),
    MsgHandler(msgCtrl,objId),
    pinOut(pinOut),
    pinIn(0),
    PtIn(PtIn),
    config(config)
{

}

CableView::~CableView()
{
    if(pinOut)
        pinOut->RemoveCable(this);
    if(pinIn)
        pinIn->RemoveCable(this);
}

void CableView::UpdatePosition(const PinView *pin, const float angle, const QPointF &pt)
{
    //move one end of the cable
    if(pin == pinOut) {
        PtOut=mapFromScene(pt);
    } else {
        PtIn=mapFromScene(pt);
    }
#ifndef SIMPLE_CABLES
    if(pin == pinOut) {
        CtrlPtOut = PtOut;
        CtrlPtOut.rx()+=50*qCos(angle);
        CtrlPtOut.ry()+=50*qSin(angle);
    } else {
        CtrlPtIn = PtIn;
        CtrlPtIn.rx()+=50*qCos(angle);
        CtrlPtIn.ry()+=50*qSin(angle);
    }

    QPainterPath newPath;
    newPath.moveTo(PtOut);
    newPath.cubicTo(CtrlPtOut,CtrlPtIn,PtIn);

    setPath(newPath);
#else
    Q_UNUSED(angle)
    setLine(PtOut.x(), PtOut.y(), PtIn.x(), PtIn.y());
#endif

}

void CableView::UpdatePosition(const QPointF &pt)
{
    PtIn=mapFromScene(pt);
#ifndef SIMPLE_CABLES
    QPainterPath newPath;
    newPath.moveTo(PtOut);
    newPath.cubicTo(CtrlPtOut,PtIn,PtIn);
    setPath(newPath);
#else
    setLine(PtOut.x(), PtOut.y(), PtIn.x(), PtIn.y());
#endif
}

void CableView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==ColorGroups::Panel && colorId==Colors::Lines) {
        setPen(color);
    }
}

void CableView::ReceiveMsg(const MsgObject &msg)
{
    QPen p = pen();

    if(msg.prop.contains(MsgObject::Delay) && msg.prop[MsgObject::Delay].toInt()>0) {
        p.setWidth(3);
    } else {
        p.setWidth(0);
    }
    setPen(p);
}
