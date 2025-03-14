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

#include "containercontent.h"
#include "objectview.h"
#include "maincontainerview.h"
#include "pinview.h"
#include "cableview.h"

// #ifdef _MSC_VER
// #pragma warning( disable: 4100 )
// #endif

using namespace View;

ContainerContent::ContainerContent(ViewConfig *config, MsgController *msgCtrl, int objId, MainContainerView * parent ) :
    ObjectDropZone(msgCtrl,objId,parent),
    config(config),
    rectAttachLeft(0),
    rectAttachRight(0)

{

    // qApp->installEventFilter(this);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setGeometry(0,0,50,50);

    dropPos.setX(0);
    dropPos.setY(0);

//    QPalette pal(palette());
//    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Object,Colors::HighlightBackground) );
//    setPalette( pal );


    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );

    HighlightStop();
}



// bool ContainerContent::eventFilter(QObject *obj, QEvent *event)
// {
//     if(event->type() == QEvent::DragEnter) {
//         // LOG("Enter");
//         QDragMoveEvent *mouseEvent = static_cast<QDragMoveEvent*>(event);
//         LOG(QString("Mouse enter (%1,%2)").arg(mouseEvent->position().x()).arg(mouseEvent->position().y()));
//     } else if(event->type() == QEvent::DragMove) {
//         // LOG("DropMove");
//         QDragMoveEvent *mouseEvent = static_cast<QDragMoveEvent*>(event);
//         LOG(QString("Mouse move (%1,%2)")  .arg(mouseEvent->position().x()) .arg(mouseEvent->position().y())  );
//         QObject * src = mouseEvent->source();
//         LOG(src->objectName())
//     } else if(event->type() == QEvent::Drop) {
//         // LOG("Drop");
//     } else if(event->type() == QEvent::GraphicsSceneDrop)   {
//         // LOG("Scene drop");
//     }
//     return QObject::eventFilter(obj, event);
// }

void ContainerContent::ReceiveMsg(const MsgObject &/*msg*/)
{

}

//void ContainerContent::SetModelIndex(QPersistentModelIndex index)
//{
//    objIndex = index;
//}

QPointF ContainerContent::GetDropPos()
{
    QPointF p = dropPos;
    dropPos.rx()+=20;
    dropPos.ry()+=20;
    return p;
}

void ContainerContent::SetDropPos(const QPointF &pt)
{
    dropPos=pt;
}

void ContainerContent::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
    // LOG("dragenter")
    //update connecting cable position
    if(event->mimeData()->hasFormat("application/x-pin")) {
        if(PinView::currentLine && PinView::currentLine->scene()==scene()) {
            PinView::currentLine->setVisible(true);
            event->accept();
        } else {
            event->ignore();
        }
        QGraphicsWidget::dragEnterEvent(event);
        return;
    }

    // QString fName;
    // QFileInfo info;


    ObjectDropZone::dragEnterEvent(event);
}

void ContainerContent::dragMoveEvent( QGraphicsSceneDragDropEvent *event)
{
    // LOG("dragmove")
    //update connecting cable position
    if(event->mimeData()->hasFormat("application/x-pin")) {
        if(PinView::currentLine) {
            PinView::currentLine->UpdatePosition( mapToScene(event->pos()) );
            PinView::currentLine->setVisible(true);
            event->ignore();
        }
        QGraphicsWidget::dragMoveEvent(event);
        return;
    }
    ObjectDropZone::dragMoveEvent(event);
}

void ContainerContent::dragLeaveEvent( QGraphicsSceneDragDropEvent *event)
{
    if(PinView::currentLine) {
        PinView::currentLine->setVisible(false);
    }
    ObjectDropZone::dragLeaveEvent(event);
}

//void ContainerContent::dropEvent( QGraphicsSceneDragDropEvent *event)
//{
//    HighlightStop();

//    MsgObject msg;
//    TranslateMimeData(event->mimeData(), msg);

////    ObjectDropZone::dropEvent(event);
//    dropPos = event->scenePos();


////    event->setAccepted(model->dropMimeData(event->mimeData(), event->proposedAction(), 0, 0, objIndex));
//}

void ContainerContent::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &/*color*/)
{
    if(groupId==ColorGroups::Panel && colorId==Colors::Background) {
        HighlightStop();
        return;
    }
}

void ContainerContent::HighlightStart()
{
    setAutoFillBackground( true );
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Panel,Colors::HighlightBackground) );
    setPalette( pal );
}

void ContainerContent::HighlightStop()
{
    QColor color = config->GetColor(ColorGroups::Panel,Colors::Background);
    //QColor color = QColor(100,100,100,100);
    if(color.alpha()==0) {
        setAutoFillBackground( false );
    } else {
        setAutoFillBackground( true );
        QPalette pal(palette());
        pal.setColor(QPalette::Window, color );
        setPalette( pal );
    }
}
