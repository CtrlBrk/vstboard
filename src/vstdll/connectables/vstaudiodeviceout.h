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

#ifndef VSTAUDIODEVICEOUT_H
#define VSTAUDIODEVICEOUT_H

//#include "precomp.h"
#include "connectables/object.h"
#include "connectables/objectinfo.h"
// #include "pluginterfaces/vst/ivstaudioprocessor.h"

// #include "views/audiograph.h"

// class AudioBuffer;

namespace Connectables {

    class VstAudioDeviceOut : public Object
    {
    Q_OBJECT
    public:
        VstAudioDeviceOut(MainHost *myHost, int index, const ObjectInfo &info);
        ~VstAudioDeviceOut();

        bool Open();
        bool Close();

        void GetBuffers(float **buf, int sampleFrames);
        void GetBuffersD(double **buf, int sampleFrames);

    protected:
        // View::AudioGraph *audiograph;

    public slots:
        void SetBufferSize(qint32 size);
    };

}

#endif // VSTAUDIODEVICEOUT_H
