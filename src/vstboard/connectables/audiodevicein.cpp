/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#include "connectables/audiodevicein.h"
#include "connectables/audiodevice.h"
#include "globals.h"
#include "audiobuffer.h"
#include "mainhosthost.h"
#include "audiodevices.h"

using namespace Connectables;

/*!
  \class Connectables::AudioDeviceIn
  \brief an audio device input. user by AudioDevice
  */

/*!
  \param myHost pointer to the MainHost
  \param index object number
  \param info object description
  */
AudioDeviceIn::AudioDeviceIn(MainHost *myHost,int index, const ObjectInfo &info) :
    Object(myHost,index, info),
    parentDevice(0)
{
    listParameterPinOut->AddPin(0);
}

AudioDeviceIn::~AudioDeviceIn()
{
    Close();
}

bool AudioDeviceIn::Close()
{
    QMutexLocker l(&objMutex);
    if(parentDevice) {
        parentDevice->SetObjectInput(0);
        parentDevice=0;
    }
    return true;
}

void AudioDeviceIn::Render()
{
    foreach(Pin* pin,listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->SendAudioBuffer();
    }

    QMutexLocker l(&objMutex);
    if(!parentDevice)
        return;



    ParameterPinOut* pin=static_cast<ParameterPinOut*>(listParameterPinOut->listPins.value(0));
    if(pin)
        pin->ChangeValue(parentDevice->GetCpuUsage());

}

void AudioDeviceIn::SetSleep(bool sleeping)
{
    Object::SetSleep(sleeping);
}

void AudioDeviceIn::SetParentDevice( AudioDevice *device )
{
    QMutexLocker l(&objMutex);
    parentDevice=device;
}

bool AudioDeviceIn::Open()
{
    closed=false;
    SetErrorMessage("");

    listAudioPinOut->EnableVuUpdates(false);

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
        parentDevice->SetObjectInput(this);
        SetErrorMessage(msg);
        return true;
    }

    //if no input channels
    if(parentDevice->GetNbInputs()==0) {
        parentDevice=0;
        //should be deleted : return false
        return false;
    }

    //enable dummy pins
    listAudioPinOut->EnableVuUpdates(true);
    //add new pins
    listAudioPinOut->ChangeNumberOfPins( parentDevice->GetNbInputs() );

    //device already has a child
    if(!parentDevice->SetObjectInput(this)) {
        parentDevice=0;
        SetErrorMessage( tr("Device already in use") );
        return true;
    }

    Object::Open();
    return true;
}

Pin* AudioDeviceIn::CreatePin(const ConnectionInfo &info)
{
    Pin *newPin = Object::CreatePin(info);
    if(newPin) {
        if(info.type==PinType::Audio)
            newPin->setObjectName(QString("Input %1").arg(info.pinNumber));
        return newPin;
    }

    switch(info.direction) {
        case PinDirection::Output :
            if(info.pinNumber==0) {
                ParameterPinOut *pin = new ParameterPinOut(this,0,0,"cpu%");
                pin->SetLimitsEnabled(false);
                return pin;
            }
            break;

        default :
            LOG("wrong PinDirection");
            return 0;
            break;
    }

    return 0;
}

#ifdef CIRCULAR_BUFFER
void AudioDeviceIn::SetBufferFromRingBuffer(QList<CircularBuffer*>listCircularBuffers)
{
    unsigned long hostBuffSize = myHost->GetBufferSize();

    int cpt=0;
    foreach(CircularBuffer *buf, listCircularBuffers) {
        AudioBuffer *pinBuf = listAudioPinOut->GetBuffer(cpt);
        cpt++;
        if(!pinBuf)
            continue;

        if(pinBuf->GetSize() < hostBuffSize) {
            LOG("resize buffer for" << objectName() );
            pinBuf->SetSize(hostBuffSize);
        }

        if(buf->filledSize >= hostBuffSize) {
            if(pinBuf->GetDoublePrecision())
                buf->Get( (double*)pinBuf->GetPointerWillBeFilled(), hostBuffSize );
            else
                buf->Get( (float*)pinBuf->GetPointerWillBeFilled(), hostBuffSize );
        }
    }
}
#endif
