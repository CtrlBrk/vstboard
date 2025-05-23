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

#ifndef AUDIODEVICEIN_H
#define AUDIODEVICEIN_H

//#include "precomp.h"
#include "connectables/object.h"
#include "connectables/objectinfo.h"
#ifdef CIRCULAR_BUFFER
    #include "circularbuffer.h"
#endif

class AudioBuffer;

namespace Connectables {

    class AudioDevice;
    class AudioDeviceIn : public Object
    {
    Q_OBJECT
    public:
        AudioDeviceIn(MainHost *myHost,int index, const ObjectInfo &info);
        ~AudioDeviceIn();
        bool Open() override;
        bool Close() override;
        void Render() override;
        Pin* CreatePin(const ConnectionInfo &info) override;
        void SetParentDevice( AudioDevice *device );
#ifdef CIRCULAR_BUFFER
        void SetBufferFromRingBuffer(QList<CircularBuffer*>listCircularBuffers);
#endif
        void NewRenderLoop() override {}
        void NewRenderLoop2() {Object::NewRenderLoop();}
        void SetSleep(bool sleeping) override;

    protected:
        /// pointer to the linked AudioDevice
        AudioDevice *parentDevice;

//    friend class AudioDevice;
    public slots:
        void SetSampleRate(float rate) override;
    };
}

#endif // AUDIODEVICEIN_H
