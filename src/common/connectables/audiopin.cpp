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

#include "audiopin.h"

#include "pin.h"
#include "object.h"
#include "../globals.h"


using namespace Connectables;

/*!
  \class Connectables::AudioPin
  \brief base class for audio pins
  */

AudioPin::AudioPin(const pinConstructArgs &conf) :
    Pin(conf),
    doublePrecision(conf.doublePrecision),
    buffer(0)
{
    buffer = new AudioBuffer(doublePrecision,conf.externalAllocation);
    SetBufferSize(conf.bufferSize);

    if(conf.direction==PinDirection::Input)
        setObjectName(QString("AudioIn%1").arg(conf.pinNumber));
    else
        setObjectName(QString("AudioOut%1").arg(conf.pinNumber));
}

/*!
  Destructor
  */
AudioPin::~AudioPin()
{
    if(buffer)
        delete buffer;
}


void AudioPin::NewRenderLoop()
{
    buffer->ResetStackCounter();
}

/*!
  set the buffer precision
  \param dblp true for double, false for float
  \return true on success
  */
bool AudioPin::SetDoublePrecision(bool dblp)
{
    if(dblp) {
        displayedText=objectName()+"=D=";
    } else {
        displayedText=objectName()+"=S=";
    }

    if(dblp==doublePrecision)
        return true;

    doublePrecision=dblp;
    buffer->SetDoublePrecision(dblp);

//    if(doublePrecision) {
//        if(!bufferD) {
//            LOG("no double precision buffer : can't convert to single precision");
//            return false;
//        }

//        if(!buffer)
//            buffer = new AudioBuffer(bufferD->IsExternallyAllocated());

//        buffer->SetSize(bufferD->GetSize());
//        buffer->ResetStackCounter();
//        buffer->AddToStack(bufferD);
//        delete bufferD;
//        bufferD = 0;
//    } else {
//        if(!buffer) {
//            LOG("no single precision buffer : can't convert to double precision");
//            return false;
//        }

//        if(!bufferD)
//            bufferD = new AudioBufferD(buffer->IsExternallyAllocated());

//        bufferD->SetSize(buffer->GetSize());
//        bufferD->ResetStackCounter();
//        bufferD->AddToStack(buffer);
//        delete buffer;
//        buffer = 0;
//    }


    return true;
}

/*!
  Resize the current buffer
  \param size the new size
  */
void AudioPin::SetBufferSize(qint32 size)
{
    buffer->SetSize(size);
}

float AudioPin::GetValue()
{
    value=buffer->GetVu();

    if(value>0.01) {
        valueChanged=true;
    }
    return value;
}

void AudioPin::ReceivePinMsg(const PinMessage::Enum msgType,void *data)
{
    if(msgType==PinMessage::AudioBuffer) {
        AudioBuffer *buf = static_cast<AudioBuffer*>(data);
        buffer->AddToStack(buf);
    }

    if(msgType==PinMessage::FadeOut) {
       // LOG("fade")
    }
}

/*!
  Send the current buffer to all connected pins
  */
void AudioPin::SendAudioBuffer()
{
    SendMsg(PinMessage::AudioBuffer,(void*)buffer);
}

void AudioPin::ReceiveMsg(const MsgObject &/*msg*/)
{
    if(buffer) {
        buffer->AddGraph();
    }
}
