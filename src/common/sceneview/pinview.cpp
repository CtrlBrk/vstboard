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

#include "pinview.h"
#include "cableview.h"
#include "../connectables/pin.h"
#include "objectview.h"

using namespace View;

CableView *PinView::currentLine = 0;

/*!
  \class View::PinView
  \brief base class for the pins views
  */

/*!
  \param angle angle in rad (0=output, pi=input) used by CableView
  \param msgCtrl pointer to the message controller
  \param parent pointer to the parent object view
  \param pinInfo description of the pin
    */
PinView::PinView(int listPinId, float angle, MsgController *msgCtrl, int objId, QGraphicsItem * parent, const ConnectionInfo &pinInfo, ViewConfig *config) :
    QGraphicsWidget(parent),
    MsgHandler(msgCtrl,objId),
    listPinId(listPinId),
    outline(0),
    highlight(0),
    connectInfo(pinInfo),
    pinAngle(angle),
    config(config),
    defaultCursor(Qt::OpenHandCursor)
{
    setAcceptDrops(true);
    setCursor(defaultCursor);
    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)) ,
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );

    actDel = new QAction( QIcon::fromTheme("process-stop") ,tr("Remove"),this);
    actDel->setShortcutContext(Qt::WidgetShortcut);
    connect(actDel,SIGNAL(triggered()),
            this,SLOT(RemovePin()));

    if(connectInfo.isRemoveable)
        addAction(actDel);

    actUnplug = new QAction( QIcon::fromTheme("edit-cut") ,tr("Unplug"),this);
    actUnplug->setShortcutContext(Qt::WidgetShortcut);
    actUnplug->setEnabled(false);
    connect(actUnplug,SIGNAL(triggered()),
            this,SLOT(Unplug()));
    addAction(actUnplug);

    UpdateKeyBinding();
    connect(config->keyBinding, SIGNAL(BindingChanged()),
            this, SLOT(UpdateKeyBinding()));
    setFocusPolicy(Qt::WheelFocus);
}

PinView::~PinView()
{
    foreach(CableView *cab, connectedCables) {
        delete cab;
    }
}

void PinView::UpdateKeyBinding()
{
    if(actDel) actDel->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::deletePin) );
    if(actUnplug) actUnplug->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::unplugPin) );
}

/*!
  Reimplements QGraphicsWidget::contextMenuEvent \n
  create a menu with all the actions
  */
void PinView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(actions().size()==0)
        return;

    QMenu menu;
    menu.exec(actions(),event->screenPos(),actions().at(0),event->widget());
}

/*!
  Update the position of all connected cables, called by PinView::itemChange and ListPinsView::UpdateCablesPosition
  */
void PinView::UpdateCablesPosition()
{
    foreach (CableView *cable, connectedCables) {
        cable->UpdatePosition(this, pinAngle, mapToScene(pinPos()) );
    }
}

/*!
  Reimplements QGraphicsWidget::itemChange \n
  calls PinView::UpdateCablesPosition when needed
  */
QVariant PinView::itemChange ( GraphicsItemChange change, const QVariant & value )
{
    if(!scene())
        return QGraphicsWidget::itemChange(change, value);

    if(change == QGraphicsItem::ItemScenePositionHasChanged)
        UpdateCablesPosition();

    return QGraphicsWidget::itemChange(change, value);
}

/*!
  Reimplements QGraphicsWidget::mousePressEvent \n
  set the closed hand cursor
  */
void PinView::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::createCable);
    if(b.input == KeyBind::mouse && b.buttons == event->buttons() && b.modifier == event->modifiers()) {
        startDragMousePos = event->screenPos();
        setCursor(defaultCursor);
        event->accept();
        return;
    }

    event->ignore();
}

/*!
  Reimplements QGraphicsWidget::mouseMoveEvent \n
  Starts a drag&drop when the conditions are met
  */
void PinView::mouseMoveEvent ( QGraphicsSceneMouseEvent  * event )
{
    if (QLineF(event->screenPos(), startDragMousePos).length() < QApplication::startDragDistance()) {
        return;
    }

    QDrag *drag = new QDrag(event->widget());

    QMimeData *mime = new QMimeData;

    QByteArray bytes;
    CreateMimeData(bytes);
    mime->setData("application/x-pin",bytes);
    drag->setMimeData(mime);

    if(!currentLine) {
        currentLine = new CableView(msgCtrl,-1,this,event->pos(),this,config);
        AddCable(currentLine);
    }

    drag->exec(Qt::CopyAction);

    if(currentLine) {
        RemoveCable(currentLine);
        currentLine->deleteLater();
        currentLine = 0;
    }
    setCursor(defaultCursor);
}

/*!
  Reimplements QGraphicsWidget::mouseReleaseEvent \n
  back to the open hand cursor
  */
void PinView::mouseReleaseEvent ( QGraphicsSceneMouseEvent  * /*event*/ )
{
    setCursor(Qt::PointingHandCursor);
}

void PinView::Unplug()
{
    MsgObject msg(connectInfo.container);
    msg.prop[MsgObject::UnplugPin]=GetIndex();
    connectInfo.GetInfos(msg);
    msgCtrl->SendMsg(msg);
}

void PinView::RemovePin()
{
    if(!connectInfo.isRemoveable)
        return;

    MsgObject msg(listPinId);
    msg.prop[MsgObject::RemovePin]=GetIndex();
    connectInfo.GetInfos(msg);
    msgCtrl->SendMsg(msg);
}

/*!
  Reimplements QGraphicsWidget::dragMoveEvent \n
  if the dragged object is connectable with us : create a temporary cable and highlight the pin
  */
void PinView::dragEnterEvent ( QGraphicsSceneDragDropEvent * event )
{
    if(event->mimeData()->hasFormat("application/x-pin")) {
        QByteArray bytes = event->mimeData()->data("application/x-pin");
        ConnectionInfo otherConnInfo;
        ReadMimeData(bytes,otherConnInfo);

        if(!connectInfo.CanConnectTo(otherConnInfo)) {
            event->ignore();
            return;
        }

        event->acceptProposedAction();

        if(currentLine) {
            currentLine->UpdatePosition(this,pinAngle,mapToScene(pinPos()));
            currentLine->setVisible(true);
        }
        if(highlight)
            highlight->setVisible(true);
    } else {
        event->ignore();
    }
}

void PinView::dragMoveEvent ( QGraphicsSceneDragDropEvent * /*event*/ )
{
    if(currentLine)
        currentLine->setVisible(true);
}

/*!
  Reimplements QGraphicsWidget::dragLeaveEvent \n
  hide the temporary cable, remove highlight on the pin
  */
void PinView::dragLeaveEvent( QGraphicsSceneDragDropEvent  * /*event*/ )
{
    if(highlight)
        highlight->setVisible(false);
    if(currentLine)
        currentLine->setVisible(false);
}

/*!
  Reimplements QGraphicsWidget::dropEvent \n
  a connectable pin was dropped on us, remove the highlight, emit a ConnectPins
  */
void PinView::dropEvent ( QGraphicsSceneDragDropEvent  * event )
{
    if(highlight)
        highlight->setVisible(false);
    QByteArray bytes = event->mimeData()->data("application/x-pin");
    ConnectionInfo connInfo;
    ReadMimeData(bytes,connInfo);

    MsgObject msg(connectInfo.container);
    msg.prop[MsgObject::ConnectPin]=1;

    MsgObject msgPin1;
    connectInfo.GetInfos(msgPin1);
    msg.children << msgPin1;

    MsgObject msgPin2;
    connInfo.GetInfos(msgPin2);
    msg.children << msgPin2;

    msgCtrl->SendMsg(msg);

//    emit ConnectPins(connectInfo, connInfo);
}

/*!
  Get the pin connection point
  \return the QPointF
  */
const QPointF PinView::pinPos() const
{
    qreal x = 0;
    if(connectInfo.direction==PinDirection::Output)
        x = geometry().width();
    return QPointF(x,geometry().height()/2);
}

/*!
  Put the connectInfo in a QByteArray
  \param[out] bytes the byte array to fill
  */
void PinView::CreateMimeData(QByteArray &bytes)
{
    QDataStream stream(&bytes,QIODevice::WriteOnly);
    stream << connectInfo.container;
    stream << connectInfo.objId;
    stream << (quint8)connectInfo.type;
    stream << (quint8)connectInfo.direction;
    stream << connectInfo.pinNumber;
    stream << connectInfo.bridge;
}

/*!
  Read a ConnectionInfo from a QByteArray
  \param[in] bytes contains the mime data
  \param[out] data ConnectionInfo to fill
  */
void PinView::ReadMimeData(QByteArray &bytes, ConnectionInfo &data)
{
    QDataStream stream(&bytes,QIODevice::ReadOnly);
    stream >> data.container;
    stream >> data.objId;
    stream >> (quint8&)data.type;
    stream >> (quint8&)data.direction;
    stream >> data.pinNumber;
    stream >> data.bridge;
}

/*!
  Add a cable to the list of connected cables and update its position
  \param cable pointer to the cable
  */
void PinView::AddCable(CableView *cable)
{
    cable->UpdatePosition(this, pinAngle, mapToScene(pinPos()) );
    connectedCables.append(cable);
    actUnplug->setEnabled(true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
}

/*!
  Remove a cable from the list of connected cables
  */
void PinView::RemoveCable(CableView *cable)
{
    connectedCables.removeAll(cable);
    if(connectedCables.isEmpty()) {
        setFlag(QGraphicsItem::ItemSendsScenePositionChanges, false);
        actUnplug->setEnabled(false);
    }
}

void PinView::ReceiveMsg(const MsgObject &msg)
{

}


