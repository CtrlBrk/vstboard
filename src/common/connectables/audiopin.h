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

#ifndef AUDIOPIN_H
#define AUDIOPIN_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

#include "pin.h"
#include "../audiobuffer.h"

namespace Connectables {

    class AudioPin : public Pin
    {
        Q_OBJECT
        Q_PROPERTY(AudioBuffer * buffer READ GetBuffer WRITE SetBuffer)

    public:
        AudioPin(Object *parent, PinDirection::Enum direction, int number, unsigned long bufferSize, bool doublePrecision=false, bool externalAllocation=false);
        virtual ~AudioPin();

//        float GetVal() { return buffer->GetCurrentVu(); }

        void SetBufferSize(unsigned long size);
        bool SetDoublePrecision(bool dblp);

        /// \return pointer to the current buffer
        AudioBuffer * GetBuffer() {return buffer;}
        void SetBuffer(AudioBuffer *buf) { buffer->AddToStack(buf); }

        float GetValue();
        void NewRenderLoop();

        void ReceivePinMsg(const PinMessage::Enum msgType,void *data=0);
        void SendAudioBuffer();

    protected:

        /// true if double precision buffer
        bool doublePrecision;

        AudioBuffer *buffer;
    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // AUDIOPIN_H
