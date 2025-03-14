/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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

#ifndef VSTMIDIDEVICE_H
#define VSTMIDIDEVICE_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning (disable: 4200)
#endif
#include "pluginterfaces/vst/ivstevents.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

//#include "precomp.h"
#include "connectables/object.h"

#define QUEUE_SIZE 1024

using namespace Steinberg;

namespace Connectables {

    class VstMidiDevice : public Object
    {
    Q_OBJECT

    public:
        VstMidiDevice(MainHost *myHost, int index, const ObjectInfo &info);
        ~VstMidiDevice();
        bool Open();
        bool Close();
        void Render();
        void MidiMsgFromInput(long msg);
		void EventFromInput(const Vst::Event &event);

        QList<long>midiQueue;
		//QList<Vst::Event>eventQueue;

    signals:
        void SendMsg(int msgType,void *data=0);
    };

}
#endif // VSTMIDIDEVICE_H
