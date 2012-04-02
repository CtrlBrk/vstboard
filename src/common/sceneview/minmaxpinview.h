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

#ifndef MINMAXPINVIEW_H
#define MINMAXPINVIEW_H

#include "connectablepinview.h"
#include "cursorview.h"

namespace View {
    class MinMaxPinView : public ConnectablePinView
    {
Q_OBJECT
    public:
        MinMaxPinView(int listPinId, float angle,  MsgController *msgCtrl,int objId, QGraphicsItem *parent, const ConnectionInfo &pinInfo, ViewConfig *config);
        void SetCursor(ObjType::Enum type, QPersistentModelIndex index);
        void UpdateLimitModelIndex(const QModelIndex &index);
        void ValueChanged(float newVal);
        void ReceiveMsg(const MsgObject &msg);

    protected:
        void AddCursor(const MsgObject &msg);

        void resizeEvent ( QGraphicsSceneResizeEvent * event );
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent ( QGraphicsSceneMouseEvent  * event );
        void mouseReleaseEvent ( QGraphicsSceneMouseEvent  * event );
        void keyPressEvent ( QKeyEvent * event );
        void wheelEvent ( QGraphicsSceneWheelEvent * event );
        void CreateCursors();
        void UpdateScaleView();
        CursorView *inMin;
        CursorView *inMax;
        CursorView *outMin;
        CursorView *outMax;
        QGraphicsPolygonItem *scaledView;
//        bool cursorCreated;
        bool changingValue;
        float startDragValue;
        QPointF startDragPos;
        ObjType::Enum parentObjType;

    public slots:
        void CursorValueChanged();
    };
}
#endif // MINMAXPINVIEW_H
