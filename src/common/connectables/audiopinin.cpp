/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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

#include "audiopinin.h"
#include "pin.h"
#include "object.h"
#include "../globals.h"
#include "../audiobuffer.h"
#include "../audiobufferd.h"

using namespace Connectables;

AudioPinIn::AudioPinIn(Object *parent, int number, bool externalAllocation, bool bridge)
    :Pin(parent,PinType::Audio,PinDirection::Input,number,bridge)
{
    doublePrecision=true;
    buffer = new AudioBuffer(externalAllocation);
    bufferD = new AudioBufferD(externalAllocation);
    setObjectName(QString("In%1").arg(number));
    visible=true;
}

AudioPinIn::~AudioPinIn()
{
    delete buffer;
    delete bufferD;
}

void AudioPinIn::NewRenderLoop()
{
    if(doublePrecision) {
        bufferD->ResetStackCounter();
    } else {
        buffer->ResetStackCounter();
    }
}

void AudioPinIn::SetBuffer(AudioBuffer *buff)
{
    buffer = buff;
}
void AudioPinIn::SetBuffer(AudioBufferD *buff)
{
    bufferD = buff;
}

void AudioPinIn::ReceiveMsg(const PinMessage::Enum msgType,void *data)
{
    if(msgType==PinMessage::AudioBuffer) {
        AudioBuffer *buf = static_cast<AudioBuffer*>(data);
        if(doublePrecision) {
            bufferD->AddToStack(buf);
        } else {
            buffer->AddToStack(buf);
        }
    }
    if(msgType==PinMessage::AudioBufferD) {
        AudioBufferD *buf = static_cast<AudioBufferD*>(data);
        if(doublePrecision) {
            bufferD->AddToStack(buf);
        } else {
            buffer->AddToStack(buf);
        }
    }
}

float AudioPinIn::GetValue()
{
    float newVu=.0f;
    if(doublePrecision)
        newVu=bufferD->GetVu();
    else
        newVu=buffer->GetVu();

    if(newVu != value) {
        valueChanged=true;
    }
    return newVu;
}

