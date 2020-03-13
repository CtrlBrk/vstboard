/**************************************************************************
#    Copyright 2010-2012 RaphaÃ«l FranÃ§ois
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

#ifndef CABLEVIEW_H
#define CABLEVIEW_H

#include "precomp.h"
#include "globals.h"
#include "pinview.h"
#include "views/viewconfig.h"
#include "msghandler.h"

namespace View {

#ifndef SIMPLE_CABLES
    class CableView : public QObject, public QGraphicsPathItem, public MsgHandler
#else
    class CableView : public QObject, public QGraphicsLineItem, public MsgHandler
#endif
    {
    Q_OBJECT
    public:
        CableView(MsgController *msgCtrl, int objId, PinView *pinOut, PinView *pinIn, QGraphicsItem *parent, ViewConfig *config);
        CableView(MsgController *msgCtrl, int objId, PinView *pinOut, const QPointF &PtIn, QGraphicsItem *parent, ViewConfig *config);
        virtual ~CableView();
        void ReceiveMsg(const MsgObject &msg);

        void UpdatePosition(const PinView *pin, const float angle, const QPointF &pt);
        void UpdatePosition(const QPointF &pt);

    protected:
        QPointF PtOut;
        QPointF PtIn;
        QPointF CtrlPtOut;
        QPointF CtrlPtIn;
        ViewConfig *config;
        PinView *pinOut;
        PinView *pinIn;

    public slots:
        void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
    };
}

#endif // CABLEVIEW_H
