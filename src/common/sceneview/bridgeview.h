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

#ifndef BRIDGEVIEW_H
#define BRIDGEVIEW_H

#include "objectview.h"
#include "cableview.h"

class MainHost;
namespace View {
    class BridgeView : public ObjectView
    {
    Q_OBJECT
    public:
        BridgeView(ViewConfig *config,MsgController *msgCtrl, int objId, QGraphicsItem * parent = 0);

    protected:
        void moveEvent ( QGraphicsSceneMoveEvent * event );

        QGraphicsLinearLayout *lLayout;

    public slots:
        void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
    };
}
#endif // BRIDGEVIEW_H
