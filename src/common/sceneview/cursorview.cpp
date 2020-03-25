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

#include "cursorview.h"
#include "globals.h"
#include "views/viewconfig.h"

using namespace View;

#define CURS_WIDTH 5.0f
#define CURD_HEIGHT 7.5f

CursorView::CursorView(MsgController *msgCtrl, int objId, bool isMaxi,bool upsideDown,QGraphicsItem *parent, ViewConfig *config) :
    QGraphicsWidget(parent),
    MsgHandler(msgCtrl,objId),
    isMaxi(isMaxi),
    upsideDown(upsideDown),
    drag(false),
    value(.0f),
    offset(QPointF(0,0)),
    config(config),
	startDragValue(.0f)
{
    QPolygonF pol;

    if(upsideDown) {
        if(isMaxi) {
            pol << QPointF(CURS_WIDTH,CURD_HEIGHT) << QPointF(0,CURD_HEIGHT) << QPointF(CURS_WIDTH,0);
            offset.setX(CURS_WIDTH);
            offset.setY(CURD_HEIGHT);
        } else {
            pol << QPointF(CURS_WIDTH,CURD_HEIGHT) << QPointF(0,CURD_HEIGHT) << QPointF(0,0);
            offset.setY(CURD_HEIGHT);
        }
    } else {
        if(isMaxi) {
            pol << QPointF(0,0) << QPointF(CURS_WIDTH,0) << QPointF(CURS_WIDTH,CURD_HEIGHT);
            offset.setX(CURS_WIDTH);
        } else {
            pol << QPointF(0,0) << QPointF(CURS_WIDTH,0) << QPointF(0,CURD_HEIGHT);
        }
    }
    cursor = new QGraphicsPolygonItem(pol,this);
    cursor->setPen(Qt::NoPen);
    cursor->setBrush( config->GetColor(ColorGroups::Cursor,Colors::Background) );
    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );

//    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::SplitHCursor);
    resize(cursor->boundingRect().size());
}

void CursorView::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Value)) {
        SetValue( msg.prop[MsgObject::Value].toFloat() );
        emit ValueChanged();
    }
}

void CursorView::setPos ( const QPointF & pos )
{
    QPointF p( pos);

    if(upsideDown) {
        p.setY(parentWidget()->rect().height());
    } else {
        p.setY(.0f);
    }

    p-=offset;
    QGraphicsWidget::setPos(p);
}

void CursorView::setPos ( qreal x, qreal y )
{
    if(upsideDown) {
        y=parentWidget()->rect().height();
    } else {
        y=.0f;
    }

    x-=offset.x();
    y-=offset.y();
    QGraphicsWidget::setPos(x,y);
}

void CursorView::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::changeCursorValue);
    if(b.input == KeyBind::mouse && b.buttons == event->button() && b.modifier == event->modifiers()) {
        event->accept();
        drag=true;
        startDragValue=value;
        startDragPos=mapToParent(event->pos());
        return;
    }
    QGraphicsWidget::mousePressEvent(event);
}

void CursorView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(drag) {
        float mouseSensibility = 1.0f/parentWidget()->size().width();
        if(event->modifiers() & Qt::ControlModifier)
            mouseSensibility /= 10;
        if(event->modifiers() & Qt::ShiftModifier)
            mouseSensibility /= 10;
        if(event->modifiers() & Qt::AltModifier)
            mouseSensibility /= 10;

        qreal increm = mapToParent(event->pos()).x() - startDragPos.x();
        startDragValue += mouseSensibility*increm;
        startDragValue = std::max(.0f,startDragValue);
        startDragValue = std::min(1.0f,startDragValue);

        event->accept();
        startDragPos=mapToParent(event->pos());
        ValueChanged( startDragValue );
        return;
    }
    QGraphicsWidget::mouseMoveEvent(event);
}

void CursorView::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    drag=false;
    QGraphicsWidget::mouseReleaseEvent(event);
}

void CursorView::wheelEvent ( QGraphicsSceneWheelEvent * event )
{
    const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::changeCursorValue);
    if(b.input == KeyBind::mouseWheel && b.modifier == event->modifiers()) {
        event->accept();

        int increm=1;
        if(event->delta()<0)
            increm=-1;

        ValueChanged( value + 0.1f*increm);
    }
}

void CursorView::SetValue(float newVal)
{
    if(newVal>1.0f)
        newVal=1.0f;
    if(newVal<.0f)
        newVal=.0f;

    if(value==newVal)
        return;
    value=newVal;

    setPos(value*parentWidget()->rect().width(),0);
}

void CursorView::SetModelIndex(QPersistentModelIndex index)
{
    modelIndex=index;
    SetValue(modelIndex.data(UserRoles::value).toFloat());
}

void CursorView::ValueChanged(float newVal)
{
    if(value==newVal)
        return;
    if(newVal>1.0f) newVal=1.0f;
    if(newVal<0.0f) newVal=0.0f;

    MsgObject msg(GetIndex());
    msg.prop[MsgObject::Value]=newVal;
    msgCtrl->SendMsg(msg);
}

//QVariant CursorView::itemChange(GraphicsItemChange change, const QVariant &value)
//{
//    if (change == ItemPositionChange && scene() && drag) {
//        QPointF newPos = value.toPointF();

//        if(newPos.x()<-offset.x())
//            newPos.setX(-offset.x());
//        if(newPos.x()>parentWidget()->rect().width()-offset.x())
//            newPos.setX(parentWidget()->rect().width()-offset.x());
//        if(upsideDown) {
//            newPos.setY(parentWidget()->rect().height()-offset.y());
//        } else {
//            newPos.setY(-offset.y());
//        }
//        if(parentWidget()->rect().width()!=0)
//            ValueChanged((newPos.x()+offset.x())/parentWidget()->rect().width());
//        return newPos;
//    }
//    return QGraphicsWidget::itemChange(change, value);
//}

void CursorView::keyPressEvent ( QKeyEvent * event )
{
    int k = event->key();

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

    QGraphicsWidget::keyPressEvent(event);
}

void CursorView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==ColorGroups::Cursor && colorId==Colors::Background && !hasFocus()) {
        cursor->setBrush(color);
    }

    if(groupId==ColorGroups::Cursor && colorId==Colors::HighlightBackground && hasFocus()) {
        cursor->setBrush(color);
    }
}

void CursorView::focusInEvent ( QFocusEvent * event )
{
    cursor->setBrush( config->GetColor(ColorGroups::Cursor,Colors::HighlightBackground) );
    QGraphicsWidget::focusInEvent(event);
}

void CursorView::focusOutEvent ( QFocusEvent * event )
{
    cursor->setBrush( config->GetColor(ColorGroups::Cursor,Colors::Background) );
    QGraphicsWidget::focusOutEvent(event);
}
