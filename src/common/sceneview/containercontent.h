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

#ifndef CONTAINERCONTENT_H
#define CONTAINERCONTENT_H

//#include "precomp.h"
#include "../mainwindow.h"
#include "objectdropzone.h"

class MainHost;
namespace View {
    class MainContainerView;
    class ContainerContent : public ObjectDropZone
    {
    Q_OBJECT
    public:
        explicit ContainerContent(ViewConfig *config, MsgController *msgCtrl, int objId, MainContainerView *parent);
        virtual void ReceiveMsg(const MsgObject &msg) override;
//        void SetModelIndex(QPersistentModelIndex index);
        QPointF GetDropPos();
        void SetDropPos(const QPointF &pt);

        ViewConfig *config;

    protected:
        // bool eventFilter(QObject *obj, QEvent *event);
        void dragEnterEvent( QGraphicsSceneDragDropEvent *event) override;
        void dragMoveEvent( QGraphicsSceneDragDropEvent *event) override;
        void dragLeaveEvent( QGraphicsSceneDragDropEvent *event) override;
       // void dropEvent( QGraphicsSceneDragDropEvent *event);

        QPersistentModelIndex objIndex;
        QPointF dropPos;

        QPersistentModelIndex attachLeft;
        QPersistentModelIndex attachRight;
        QGraphicsRectItem *rectAttachLeft;
        QGraphicsRectItem *rectAttachRight;
    signals:

    public slots:
        void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
        void HighlightStart() override;
        void HighlightStop() override;
    };
}
#endif // CONTAINERCONTENT_H
