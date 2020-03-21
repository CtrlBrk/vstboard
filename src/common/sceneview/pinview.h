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

#ifndef PINVIEW_H
#define PINVIEW_H

//#include "precomp.h"
#include "globals.h"
#include "connectables/connectioninfo.h"
#include "views/viewconfig.h"
#include "msghandler.h"

namespace Connectables {
    class Pin;
}

namespace View {
    class CableView;
    class PinView : public QGraphicsWidget, public MsgHandler
    {
    Q_OBJECT

    public:

        explicit PinView(int listPinId, float angle, MsgController *ctrl, int objId, QGraphicsItem * parent, const ConnectionInfo &pinInfo, ViewConfig *config);
        virtual ~PinView();
        const ConnectionInfo GetConnectionInfo() const {return connectInfo;}
        void AddCable(CableView *cable);
        void RemoveCable(CableView *cable);
        void UpdateCablesPosition();
        virtual void ReceiveMsg(const MsgObject &msg);
        virtual const QPointF pinPos() const;

        /// temporary cable for drag&drop
        static CableView *currentLine;

    protected:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
        virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
        virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent  * event );
        virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent  * event );
        void dragLeaveEvent( QGraphicsSceneDragDropEvent  * event );
        void dragEnterEvent ( QGraphicsSceneDragDropEvent * event );
        void dragMoveEvent ( QGraphicsSceneDragDropEvent * event );
        void dropEvent ( QGraphicsSceneDragDropEvent  * event );

        void CreateMimeData(QByteArray &bytes);
        void ReadMimeData(QByteArray &bytes, ConnectionInfo &data);

        /// outline
        QGraphicsRectItem *outline;

        /// highlight
        QGraphicsRectItem *highlight;

        /// list of connected cables
        QList<CableView *> connectedCables;

        /// description of the pin
        ConnectionInfo connectInfo;

        /// pin angle in rad
        float pinAngle;

        ViewConfig *config;

        QAction *actDel;
        QAction *actUnplug;

        Qt::CursorShape defaultCursor;

        QPoint startDragMousePos;

        int listPinId;

    signals:
//        void ConnectPins(ConnectionInfo outputPin, ConnectionInfo inputPin);
//        void RemoveCablesFromPin(ConnectionInfo pin);
//        void RemovePin(ConnectionInfo pin);

    public slots:
        /// update the vu-meter, called by a timer
        virtual void updateVu() {}
        virtual void UpdateColor(ColorGroups::Enum /*groupId*/, Colors::Enum /*colorId*/, const QColor & /*color*/) {}
        void RemovePin();
        void Unplug();
        void UpdateKeyBinding();

    friend class Cable;
    };
}

#endif // PINVIEW_H
