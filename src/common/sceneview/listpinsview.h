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

#ifndef LISTPINSVIEW_H
#define LISTPINSVIEW_H

//#include "../precomp.h"
#include "msghandler.h"
#include "views/viewconfig.h"
namespace View {
    class PinView;
    class ListPinsView : public QGraphicsWidget, public MsgHandler
    {
    Q_OBJECT
    public:
        ListPinsView(ViewConfig *config, MsgController *msgCtrl, int objId, QGraphicsItem * parent = 0);
        QGraphicsLinearLayout *layout;
        void UpdateCablesPosition();
        int GetPinPosition(int newPinNumber);
        void ReceiveMsg(const MsgObject &msg);
        ViewConfig *config;
    private:
        void AddPin(const MsgObject &msg);

    private slots:
        void UpdateLayout();
    signals:
        void Shrinklist();
    };
}

#endif // LISTPINSVIEW_H
