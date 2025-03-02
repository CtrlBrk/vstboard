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

#ifndef CLAPMIDIDEVICE_H
#define CLAPMIDIDEVICE_H

#include <QObject>

#include "connectables/object.h"
#include <clap/process.h>
#define QUEUE_SIZE 1024

namespace Connectables {

    class ClapMidiDevice : public Object
    {
        Q_OBJECT
    public:
        ClapMidiDevice(MainHost *myHost, int index, const ObjectInfo &info);
        ~ClapMidiDevice();
        bool Open();
        bool Close();
        void Render();
        void MidiMsgFromInput(long msg);
        void EventFromInput(const clap_input_events_t *event);

        QList<long>midiQueue;

    protected:
        void HandleEvent(const clap_event_header_t *evt);

    signals:
        void SendMsg(int msgType,void *data=0);
    };
}
#endif // CLAPMIDIDEVICE_H
