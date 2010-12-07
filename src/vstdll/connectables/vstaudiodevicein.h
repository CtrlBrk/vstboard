/******************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
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
******************************************************************************/

#ifndef VSTAUDIODEVICEIN_H
#define VSTAUDIODEVICEIN_H

#include "../vstboard/precomp.h"
#include "../vstboard/connectables/object.h"
#include "../vstboard/connectables/objectinfo.h"

class AudioBuffer;

namespace Connectables {

    class VstAudioDeviceIn : public Object
    {
    Q_OBJECT
    public:
        VstAudioDeviceIn(MainHost *myHost,int index, const ObjectInfo &info);
        ~VstAudioDeviceIn();

        bool Open();
        bool Close();
        void Render();
//        int GetProcessingTime() {return 10;}

        bool bufferReady;

        void SetBuffers(float **buf);

    protected:
//        QSharedPointer<AudioDevice>parentDevice;

    public slots:
        void SetBufferSize(long size);

//    friend class AudioDevice;
    };
}

#endif // VSTAUDIODEVICEIN_H
