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

#ifndef CONNECTABLEPINVIEW_H
#define CONNECTABLEPINVIEW_H

#include "pinview.h"
//#include "outlinedtext.h"

namespace View {

    class ConnectablePinView : public PinView
    {
    Q_OBJECT
    public:
        ConnectablePinView(int listPinId, float angle, MsgController *msgCtrl, int objId, QGraphicsItem * parent, const ConnectionInfo &pinInfo, ViewConfig *config);
        virtual void ReceiveMsg(const MsgObject &msg);
        float GetValue() {return value;}

    protected:
        void resizeEvent ( QGraphicsSceneResizeEvent * event );
        virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
        QGraphicsSimpleTextItem *textItem;
        QGraphicsRectItem *rectVu;
        QString text;
        float value;
        bool isParameter;
        ColorGroups::Enum colorGroupId;
        QColor vuColor;
        int overload;
        float vuFall;

    public slots:
        void updateVu();
        void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
        void ResetOveload();

    };

}
#endif // CONNECTABLEPINVIEW_H
