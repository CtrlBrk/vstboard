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

#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

//#include "precomp.h"
#include "portmidi.h"
#include "pmutil.h"
#include "connectables/object.h"

#define QUEUE_SIZE 1024

namespace Connectables {

    class MidiDevice : public Object
    {
    Q_OBJECT

    public:
        MidiDevice(MainHost *myHost,int index, const ObjectInfo &info);
        ~MidiDevice();
        bool Open();
        bool Close();
        void Render();
        void MidiMsgFromInput(long msg);

        PmStream *stream;
        PmQueue *queue;

        const PmDeviceInfo *devInfo;
        bool OpenStream();
        bool CloseStream();

    protected:
        bool FindDeviceByName();
        bool deviceOpened;

    signals:
        void SendMsg(int msgType,void *data=0);
    };

}
#endif // MIDIDEVICE_H
