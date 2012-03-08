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

#ifndef OBJECTDROPZONE_H
#define OBJECTDROPZONE_H

#include <QGraphicsWidget>
#include "msghandler.h"

namespace View {
    class ObjectDropZone : public QGraphicsWidget, public MsgHandler
    {
        Q_OBJECT
    public:
        explicit ObjectDropZone(MsgController *msgCtrl, int objId, QGraphicsItem *parent, QWidget *parking=0);
        QWidget *GetParking() {return myParking;}

    protected:
        void dragEnterEvent( QGraphicsSceneDragDropEvent *event);
        void dragMoveEvent( QGraphicsSceneDragDropEvent *event);
        void dragLeaveEvent( QGraphicsSceneDragDropEvent *event);
        void dropEvent( QGraphicsSceneDragDropEvent *event);
        bool TranslateMimeData( const QMimeData * data, MsgObject &msg );
        QWidget *myParking;

    signals:
        void ObjectDropped(QGraphicsSceneDragDropEvent *event, MsgObject msg);

    public slots:
        virtual void HighlightStart();
        virtual void HighlightStop();
        void SetParking(QWidget *park) {myParking=park;}
        void UpdateHeight();
    };
}
#endif // OBJECTDROPZONE_H
