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

#include "vstaudiodevicein.h"
#include "vstaudiodevice.h"
#include "globals.h"
#include "audiobuffer.h"
#include "mainhost.h"
#include "testvst.h"

using namespace Connectables;

VstAudioDeviceIn::VstAudioDeviceIn(int index, const ObjectInfo &info) :
    Object(index, info),
    bufferReady(false)
{
}

VstAudioDeviceIn::~VstAudioDeviceIn()
{
    Close();
}

bool VstAudioDeviceIn::Close()
{
    TestVst::theVstPlugin->setDeviceIn(0);
    if(!Object::Close())
        return false;
    return true;
}

void VstAudioDeviceIn::Render()
{
    foreach(AudioPinOut* pin,listAudioPinOut) {
        pin->buffer->ConsumeStack();
        pin->SendAudioBuffer();
    }
}

void VstAudioDeviceIn::SetBufferSize(long size)
{
    foreach(AudioPinIn *pin, listAudioPinIn) {
        pin->buffer->SetSize(size);
    }
    foreach(AudioPinOut *pin, listAudioPinOut) {
        pin->buffer->SetSize(size);
    }
}

bool VstAudioDeviceIn::Open()
{
    if(!TestVst::theVstPlugin->setDeviceIn(this))
        return false;

    AudioPinOut *pin=0;
    for(int i=0; i<2; i++) {
        pin = new AudioPinOut(this,i,true);
        listAudioPinOut << pin;
        pin->buffer->SetSize(512);
        pin->setObjectName(QString("Input %1").arg(i));
    }

    Object::Open();
    return true;
}

void VstAudioDeviceIn::SetBuffers(float **buf)
{
    int cpt=0;
    foreach(AudioPinOut* pin,listAudioPinOut) {
        pin->buffer->ResetStackCounter();
        pin->buffer->SetPointer(buf[cpt]);
        cpt++;
    }
}
