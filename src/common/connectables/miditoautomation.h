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

#ifndef MIDITOAUTOMATION_H
#define MIDITOAUTOMATION_H

//#include "precomp.h"
#include "object.h"

namespace Connectables {

    class MidiToAutomation : public Object
    {
    Q_OBJECT
    public:
        explicit MidiToAutomation(MainHost *myHost,int index);
        void Render();
        void MidiMsgFromInput(long msg);
        Pin* CreatePin(const ConnectionInfo &info);
    protected:
        void ChangeValue(int ctrl, int value);
        QList<QVariant> listValues;
        QHash<quint16,quint8>listChanged;

        /// list of values used by the learn pin (off, learn, unlearn)
        QList<QVariant>listIsLearning;

        enum paramNumbers {
            para_prog=128,
            para_velocity,
            para_notepitch,
            para_pitchbend,
            para_chanpress,
            para_aftertouch,
            para_notes=200
        };

    };
}

#endif // MIDITOAUTOMATION_H
