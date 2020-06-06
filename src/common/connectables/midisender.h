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

#ifndef MIDISENDER_H
#define MIDISENDER_H

//#include "../precomp.h"
#include "object.h"

namespace Connectables {

    class MidiSender : public Object
    {
    Q_OBJECT
        enum Param {
            Param_MsgType = 0,
            Param_Value1 = 1,
            Param_Value2 = 2,
            Param_Channel = 3
        };

    public:
        MidiSender(MainHost *myHost,int index);
        void Render();
        Pin* CreatePin(const ConnectionInfo &info);
    protected:
        long midiMsg;
        bool msgChanged;

        QList<QVariant> listMsgType;
        QList<QVariant> listValues;
        QList<QVariant> listChannels;

    public slots:
        virtual void OnParameterChanged(ConnectionInfo pinInfo, float value);
    };
}
#endif // MIDISENDER_H
