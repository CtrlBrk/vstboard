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

#include "connectables/audiodeviceout.h"
#ifdef RTAUDIO
#include "connectables/audiodevice_rt.h"
#include "audiodevices_rt.h"
#endif
#ifdef PORTAUDIO
#include "connectables/audiodevice_pa.h"
#include "audiodevices_pa.h"
#endif
#include "globals.h"
#include "audiobuffer.h"
#include "mainhosthost.h"

using namespace Connectables;

/*!
  \class Connectables::AudioDeviceOut
  \brief an audio device output. used by AudioDevice
  */

/*!
  \param myHost pointer to the MainHost
  \param index object number
  \param info object description
  */
AudioDeviceOut::AudioDeviceOut(MainHost *myHost,int index, const ObjectInfo &info) :
    Object(myHost,index, info),
    parentDevice(0)
{
    doublePrecision=false;
}

AudioDeviceOut::~AudioDeviceOut()
{
    Close();
}

void AudioDeviceOut::SetParentDevice( AudioDevice *device )
{
    objMutex.lock();
    parentDevice=device;
    objMutex.unlock();
}

bool AudioDeviceOut::Close()
{
    objMutex.lock();
    if(parentDevice) {
        parentDevice->SetObjectOutput(0);
        parentDevice=0;
    }
    objMutex.unlock();
    return true;
}

bool AudioDeviceOut::Open()
{
//    QMutexLocker l(&objMutex);

    closed=false;
    SetErrorMessage("");

    listAudioPinIn->EnableVuUpdates(false);

    //create the audiodevice if needed
    if(!parentDevice) {
        MainHostHost *host=static_cast<MainHostHost*>(myHost);
        parentDevice=host->audioDevices->AddDevice(objInfo);
        if(!parentDevice) {
            SetErrorMessage( tr("Device not created") );
            return true;
        }
    }

    //if parent device in error, return
    QString msg;
    parentDevice->GetErrMessage(msg);
    if(!msg.isEmpty()) {
        parentDevice->SetObjectOutput(this);
        SetErrorMessage(msg);
        return true;
    }

    //if no output channels
    if(parentDevice->GetNbOutputs()==0) {
        parentDevice=0;
        //should be deleted : return false
        return false;
    }

    //enable dummy pins
    listAudioPinIn->EnableVuUpdates(true);
    //add new pins
    listAudioPinIn->ChangeNumberOfPins( parentDevice->GetNbOutputs() );

    //debug pins
#ifndef QT_NO_DEBUG
//    listAudioPinOut->ChangeNumberOfPins( parentDevice->GetNbOutputs() );
#endif

    //device already has a child
    if(!parentDevice->SetObjectOutput(this)) {
        parentDevice=0;
        SetErrorMessage( tr("Already in use") );
        return true;
    }

    Object::Open();
    return true;
}

#ifdef CIRCULAR_BUFFER
void AudioDeviceOut::SetRingBufferFromPins(QList<CircularBuffer*>listCircularBuffers) {
//    AudioBuffer *pinBuf = listAudioPinIn->GetBuffer(0);
//    static ulong put=0;
//    put+=pinBuf->GetSize();
//    LOG(QString("put %1").arg(put));

    int cpt=0;
    foreach(CircularBuffer *buf, listCircularBuffers) {
//        if(buf->buffSize<myHost->GetBufferSize()*2)
//            buf->SetSize(myHost->GetBufferSize()*2);

        AudioBuffer *pinBuf = listAudioPinIn->GetBuffer(cpt);
        cpt++;
        if(!pinBuf) {
            LOG("no pin buffer");
            continue;
        }

//        LOG(QString("pin->buffer %1->%2/%3")
//            .arg(pinBuf->GetSize())
//            .arg(buf->filledSize)
//            .arg(buf->buffSize)
//            );


        if(pinBuf->GetDoublePrecision())
            buf->Put( (double*)pinBuf->ConsumeStack(), pinBuf->GetSize() );
        else
            buf->Put( (float*)pinBuf->ConsumeStack(), pinBuf->GetSize() );
        pinBuf->ResetStackCounter();
    }
}
#endif

void AudioDeviceOut::Render()
{
#ifndef QT_NO_DEBUG
    //for debug output pins
//    foreach(Pin *in, listAudioPinIn->listPins ) {
//        AudioPin *audioIn = static_cast<AudioPin*>(in);
//        AudioPin *audioOut = 0;
//        if(listAudioPinOut->listPins.size() > in->GetConnectionInfo().pinNumber) {
//            audioOut = static_cast<AudioPin*>(listAudioPinOut->GetPin( in->GetConnectionInfo().pinNumber ));
//        }

//        if(audioIn) {
//            if(audioOut) {
//                audioOut->GetBuffer()->AddToStack( audioIn->GetBuffer() );
//                audioOut->GetBuffer()->ConsumeStack();
//                audioOut->SendAudioBuffer();
//                audioOut->GetBuffer()->ResetStackCounter();
//            }
//            audioIn->GetBuffer()->ConsumeStack();
//        }
//    }
#endif
}
