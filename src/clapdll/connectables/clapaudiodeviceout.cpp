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

#include "clapaudiodeviceout.h"
#include "globals.h"
#include "audiobuffer.h"
#include "clapmainhost.h"

using namespace Connectables;

ClapAudioDeviceOut::ClapAudioDeviceOut(MainHost *myHost, int index, const ObjectInfo &info) :
    Object(myHost,index, info)
{
    // audiograph = new View::AudioGraph();
    // audiograph ->resize(800, 600);
    // audiograph ->show();
}

ClapAudioDeviceOut::~ClapAudioDeviceOut()
{
    Close();
    // delete audiograph;
}

bool ClapAudioDeviceOut::Close()
{
    static_cast<ClapMainHost*>(myHost)->removeAudioOut(this);
    if(!Object::Close())
        return false;
    return true;
}

void ClapAudioDeviceOut::SetBufferSize(qint32 size)
{
    foreach(Pin *pin, listAudioPinIn->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->SetSize(size);
    }
}

bool ClapAudioDeviceOut::Open()
{
    if(!static_cast<ClapMainHost*>(myHost)->addAudioOut(this)) {
        SetErrorMessage( tr("No available bus") );
        return true;
    }

    listAudioPinIn->ChangeNumberOfPins(2);
    SetBufferSize(myHost->GetBufferSize());
    Object::Open();
    return true;
}

void ClapAudioDeviceOut::GetBuffers(float **buf, int sampleFrames)
{
    int cpt=0;
    foreach(Pin *pin, listAudioPinIn->listPins) {
        AudioBuffer *abuf= static_cast<AudioPin*>(pin)->GetBuffer();
        abuf->ConsumeStack();
        abuf->DumpToBuffer(buf[cpt],sampleFrames);

        // if(cpt==0) {
        // audiograph->UpdateGraph((float*)abuf->GetPointer(),sampleFrames);
        // }

        abuf->ResetStackCounter();
        cpt++;
    }
}

void ClapAudioDeviceOut::GetBuffersD(double **buf, int sampleFrames)
{
    int cpt=0;
    foreach(Pin *pin, listAudioPinIn->listPins) {
        AudioBuffer *abuf= static_cast<AudioPin*>(pin)->GetBuffer();
        abuf->ConsumeStack();
        abuf->DumpToBuffer(buf[cpt],sampleFrames);


        abuf->ResetStackCounter();
        cpt++;
    }
}


