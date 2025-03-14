/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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

#include "connectableobjectview.h"
#include "connectablepinview.h"
#include "connectables/objectfactory.h"
#include "connectables/container.h"

using namespace View;

ConnectableObjectView::ConnectableObjectView(ViewConfig *config, MsgController *msgCtrl, int objId, MainContainerView * parent=0 ) :
    ObjectView(config,msgCtrl,objId,parent),
	objType(ObjType::ND),
    dropReplace(0),
    dropAttachLeft(0),
    dropAttachRight(0),
    moving(false)
{
    titleText = new QGraphicsSimpleTextItem(QString("Title"),this);
    titleText->moveBy(2,1);
    titleText->setBrush( config->GetColor(ColorGroups::Object,Colors::Text) );

    layout = new QGraphicsGridLayout() ;
    layout->setSpacing(0);
    layout->setHorizontalSpacing(5);
    layout->setContentsMargins(0,15,0,0);
    setLayout(layout);
    setFocusPolicy(Qt::StrongFocus);

    listAudioIn = new ListPinsView(config,msgCtrl,-1,this);
    listAudioOut = new ListPinsView(config,msgCtrl,-1,this);
    listMidiIn = new ListPinsView(config,msgCtrl,-1,this);
    listMidiOut = new ListPinsView(config,msgCtrl,-1,this);
    listParametersIn = new ListPinsView(config,msgCtrl,-1,this);
    listParametersOut = new ListPinsView(config,msgCtrl,-1,this);

    layout->addItem(listAudioIn,0,0,Qt::AlignLeft | Qt::AlignTop);
    layout->addItem(listAudioOut,0,1,Qt::AlignRight | Qt::AlignTop);
    layout->addItem(listMidiIn,1,0,Qt::AlignLeft | Qt::AlignTop);
    layout->addItem(listMidiOut,1,1,Qt::AlignRight | Qt::AlignTop);
    layout->addItem(listParametersIn,2,0,Qt::AlignLeft | Qt::AlignTop);
    layout->addItem(listParametersOut,2,1,Qt::AlignRight | Qt::AlignTop);

    connect(listParametersIn,SIGNAL(Shrinklist()),
            this,SLOT(UpdateLayout()));
    connect(listParametersOut,SIGNAL(Shrinklist()),
            this,SLOT(UpdateLayout()));

    dropReplace = new ObjectDropZone(msgCtrl,-1,this,parent->GetParking());
    connect(parent,SIGNAL(ParkingChanged(QWidget*)),
            dropReplace,SLOT(SetParking(QWidget*)));
    connect(dropReplace, SIGNAL(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)),
            this,SLOT(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)));
    connect(this, SIGNAL(heightChanged()),
            dropReplace, SLOT(UpdateHeight()));

    dropAttachLeft = new ObjectDropZone(msgCtrl,-1,this,parent->GetParking());
    connect(parent,SIGNAL(ParkingChanged(QWidget*)),
            dropAttachLeft,SLOT(SetParking(QWidget*)));
    connect(dropAttachLeft, SIGNAL(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)),
            this,SLOT(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)));
    connect(this, SIGNAL(heightChanged()),
            dropAttachLeft, SLOT(UpdateHeight()));

    dropAttachRight = new ObjectDropZone(msgCtrl,-1,this,parent->GetParking());
    connect(parent,SIGNAL(ParkingChanged(QWidget*)),
            dropAttachLeft,SLOT(SetParking(QWidget*)));
    connect(dropAttachRight, SIGNAL(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)),
            this,SLOT(ObjectDropped(QGraphicsSceneDragDropEvent*,MsgObject)));
    connect(this, SIGNAL(heightChanged()),
            dropAttachRight, SLOT(UpdateHeight()));

    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Object,Colors::HighlightBackground) );
    dropReplace->setPalette( pal );
    dropAttachLeft->setPalette( pal );
    dropAttachRight->setPalette( pal );

    setGeometry(QRectF(0,0,105,15));
}

void ConnectableObjectView::UpdateLayout()
{
    //still not shrinking after a pin is removed
    layout->invalidate();
}

void ConnectableObjectView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    ObjectView::resizeEvent(event);

    if(dropReplace)
        dropReplace->setGeometry( 20,0,size().width()-40, event->newSize().height());
    if(dropAttachLeft)
        dropAttachLeft->setGeometry(-10,0,30, event->newSize().height());
    if(dropAttachRight)
        dropAttachRight->setGeometry(event->newSize().width()-20,0,30, event->newSize().height());
}

void ConnectableObjectView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
    QGraphicsWidget::mousePressEvent(event);
    if(!event->isAccepted()) {

        const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::moveObject);
        if(b.input == KeyBind::mouse && b.modifier == event->modifiers() && b.buttons == event->buttons()) {
            event->accept();
            moveOffset=pos()-mapToParent(mapFromScene(event->scenePos()));
            startDragMousePos = event->screenPos();
            moving=true;
            return;
        }
    }

    if(moving) {
        moving=false;
//        model->setData(objIndex,pos(),UserRoles::position);
        return;
    }
}

void ConnectableObjectView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->setAccepted(false);
    QGraphicsWidget::mouseMoveEvent(event);
    if(event->isAccepted())
        return;

    if(moving && QLineF(event->screenPos(), startDragMousePos).length() > QApplication::startDragDistance()) {
        event->accept();
        setPos(mapToParent(mapFromScene(event->scenePos()))+moveOffset);
    }
}

/*!
  Reimplements QGraphicsWidget::mouseReleaseEvent \n
  update the model with the new position
  */
void ConnectableObjectView::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    if(moving) {
        moving=false;
        ReportPosChange();
        event->accept();
        return;
    }
    QGraphicsWidget::mouseReleaseEvent(event);

}

void ConnectableObjectView::ReportPosChange()
{
    MSGOBJ();
    msg.prop[MsgObject::State]=pos();
    msgCtrl->SendMsg(msg);
}

void ConnectableObjectView::ObjectDropped(QGraphicsSceneDragDropEvent *event, MsgObject msg)
{
    QPointF dropPos(0,0);

    if(sender()==dropReplace){
        msg.prop[MsgObject::Type]=InsertionType::Replace;
    }

    if(event->dropAction()==Qt::MoveAction) {
        if(sender()==dropAttachLeft) {
            msg.prop[MsgObject::Type]=InsertionType::InsertBefore;
            dropPos.rx()-=(geometry().width()+10);
        }
        if(sender()==dropAttachRight){
            msg.prop[MsgObject::Type]=InsertionType::InsertAfter;
            dropPos.rx()+=(geometry().width()+10);
        }
    } else {
        if(sender()==dropAttachLeft) {
            msg.prop[MsgObject::Type]=InsertionType::AddBefore;
            dropPos.rx()-=(geometry().width()+10);
        }
        if(sender()==dropAttachRight){
            msg.prop[MsgObject::Type]=InsertionType::AddAfter;
            dropPos.rx()+=(geometry().width()+10);
        }
    }

    MainContainerView *cnt = static_cast<MainContainerView*>(parentItem());
    if(cnt)
        cnt->SetDropPos( mapToScene(dropPos) );

    msg.objIndex=GetIndex();
    msgCtrl->SendMsg(msg);

//    event->setAccepted(model->dropMimeData(event->mimeData(), event->dropAction(), 0, col, objIndex));
}

void ConnectableObjectView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(colorId==Colors::Background)  {
        if(groupId==myColorGroupId) {
            if(!highlighted) {
                QPalette pal(palette());
                pal.setColor(QPalette::Window,color);
                setPalette( pal );
            }
        }
        return;
    }

    if(groupId==ColorGroups::Object && colorId==Colors::HighlightBackground) {
        QPalette pal(palette());
        pal.setColor(QPalette::Window, color );
        dropReplace->setPalette( pal );
        dropAttachLeft->setPalette( pal );
        dropAttachRight->setPalette( pal );
        return;
    }
    ObjectView::UpdateColor(groupId,colorId,color);
}
